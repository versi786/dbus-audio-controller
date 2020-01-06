#include <dbus/dbus-glib.h>  // for dbus_g_*
#include <dbus/dbus.h>       // for dbus_*
#include <generated_mediaplayer2.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

#define UNUSED(expr)  \
    do {              \
        (void)(expr); \
    } while (0)

struct PlayerAction {
    enum X { NONE = 0, PLAY_PAUSE, NEXT, PREV };
};

namespace logging = boost::log;

// Code  similar to
// https://github.com/altdesktop/playerctl/blob/master/playerctl/playerctl-player.c
#define MPRIS_PREFIX "org.mpris.MediaPlayer2."
static int list_player_names_on_bus(std::vector<std::string> *players) {
    GError *tmp_error = NULL;

    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                      G_DBUS_PROXY_FLAGS_NONE,
                                                      NULL,
                                                      "org.freedesktop.DBus",
                                                      "/org/freedesktop/DBus",
                                                      "org.freedesktop.DBus",
                                                      NULL,
                                                      &tmp_error);

    if (tmp_error != NULL) {
        if (tmp_error->domain == G_IO_ERROR &&
            tmp_error->code == G_IO_ERROR_NOT_FOUND) {
            // XXX: This means the dbus socket address is not found which may
            // mean that the bus is not running or the address was set
            // incorrectly. I think we can pass through here because it is true
            // that there are no names on the bus that is supposed to be at
            // this socket path. But we need a better way of dealing with this
            // case.
            BOOST_LOG_TRIVIAL(error)
                << "D-Bus socket address not found, unable to list player names"
                << std::endl;
            return -1;
        }
        return -2;
    }

    GVariant *reply = g_dbus_proxy_call_sync(
        proxy, "ListNames", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(proxy);
        return -2;
    }

    GVariant *reply_child = g_variant_get_child_value(reply, 0);
    gsize reply_count;
    const gchar **names = g_variant_get_strv(reply_child, &reply_count);

    size_t offset = strlen(MPRIS_PREFIX);
    for (gsize i = 0; i < reply_count; i += 1) {
        if (g_str_has_prefix(names[i], MPRIS_PREFIX)) {
            const char *player_name = names[i] + offset;
            players->push_back(player_name);
            BOOST_LOG_TRIVIAL(debug)
                << "found player: " << player_name << std::endl;
        }
    }

    g_object_unref(proxy);
    g_variant_unref(reply);
    g_variant_unref(reply_child);
    g_free(names);

    return 0;
}

// End code from

int send_command_to_player(const std::string &player, int playerAction) {
    // Create the proxy

    GError *error;
    error = NULL;

    std::string service("org.mpris.MediaPlayer2.");
    service += player;

    OrgMprisMediaPlayer2Player *playerProxy;
    playerProxy = org_mpris_media_player2_player_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION,
        G_DBUS_PROXY_FLAGS_NONE,
        service.c_str(),
        "/org/mpris/MediaPlayer2",
        NULL,
        &error);

    if (error) {
        BOOST_LOG_TRIVIAL(error) << error->message << std::endl;
        g_object_unref(playerProxy);
        return -1;
    }

    // send play command
    gboolean ret = false;

    switch (playerAction) {
        case PlayerAction::PLAY_PAUSE: {
            ret = org_mpris_media_player2_player_call_play_pause_sync(
                playerProxy, NULL, &error);
        } break;
        case PlayerAction::NEXT: {
            ret = org_mpris_media_player2_player_call_next_sync(
                playerProxy, NULL, &error);
        } break;
        case PlayerAction::PREV: {
            ret = org_mpris_media_player2_player_call_previous_sync(
                playerProxy, NULL, &error);
        } break;

        default: { std::cerr << "Invalid option" << std::endl; } break;
    }

    if (error || !ret) {
        BOOST_LOG_TRIVIAL(error) << error->message << std::endl;
        g_object_unref(playerProxy);
        return -1;
    }

    g_object_unref(playerProxy);
    return 0;
}

int run(int playerAction) {
    std::vector<std::string> players;
    int rc = list_player_names_on_bus(&players);
    if (rc != 0) {
        std::cerr << "Failed to list players rc: " << rc << std::endl;
    }

    for (auto player : players) {
        rc |= send_command_to_player(player, playerAction);
    }
    return rc;
}

int parse_commandline(int argc, char **argv, int *playerAction) {
    *playerAction = PlayerAction::NONE;
    bool playPause = false;
    bool next = false;
    bool prev = false;
    bool debug = false;

    using namespace boost::program_options;
    try {
        options_description desc{
            "Send the following command on all players connected to session "
            "DBus."};
        desc.add_options()("help,h", "Help screen")(
            "play-pause", bool_switch(&playPause), "Play/Pause")(
            "next", bool_switch(&next), "Next")(
            "prev", bool_switch(&prev), "Previous")(
            "debug", bool_switch(&debug), "Enable debug logging");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
        int numTrue = 0;

        if (playPause && *playerAction == PlayerAction::NONE) {
            *playerAction = PlayerAction::PLAY_PAUSE;
            ++numTrue;
        }
        if (next && *playerAction == PlayerAction::NONE) {
            *playerAction = PlayerAction::NEXT;
            ++numTrue;
        }
        if (prev && *playerAction == PlayerAction::NONE) {
            *playerAction = PlayerAction::PREV;
            ++numTrue;
        }
        if (vm.count("help") || numTrue > 1) {
            std::cout << desc << std::endl;
            if (numTrue) {
                return numTrue;
            }
            return 0;
        }

        // we did not get any command or request for printing of help,
        // print help anyway and return an error
        if (numTrue != 1) {
            std::cout << desc << std::endl;
            return -1;
        }

        // set log level
        if (debug) {
            logging::core::get()->set_filter(logging::trivial::severity >=
                                             logging::trivial::debug);
        } else {
            logging::core::get()->set_filter(logging::trivial::severity >=
                                             logging::trivial::info);
        }

    } catch (const error &ex) {
        std::cerr << ex.what() << '\n';
    }
    return 1;
}

int main(int argc, char **argv) {
    int playerAction = PlayerAction::NONE;
    int rc = 0;
    rc = parse_commandline(argc, argv, &playerAction);
    if (rc) return rc;
    if (playerAction != PlayerAction::NONE) {
        return run(playerAction);
    }
    return 0;
}

