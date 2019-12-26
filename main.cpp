#include <dbus/dbus-glib.h>  // for dbus_g_*
#include <dbus/dbus.h>       // for dbus_*
#include <generated_mediaplayer2.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <iostream>
#include <string>
#include <vector>

#define UNUSED(expr)  \
    do {              \
        (void)(expr); \
    } while (0)

// Code  similar to
// https://github.com/altdesktop/playerctl/blob/master/playerctl/playerctl-player.c

#define MPRIS_PREFIX "org.mpris.MediaPlayer2."
static int list_player_names_on_bus(std::vector<std::string> *players,
                                    GBusType bus_type,
                                    GError **err) {
    GError *tmp_error = NULL;

    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(bus_type,
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
            std::cerr
                << "D-Bus socket address not found, unable to list player names"
                << std::endl;
            return -1;
        }
        g_propagate_error(err, tmp_error);
        return -2;
    }

    GVariant *reply = g_dbus_proxy_call_sync(
        proxy, "ListNames", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        g_object_unref(proxy);
        return -2;
    }

    GVariant *reply_child = g_variant_get_child_value(reply, 0);
    gsize reply_count;
    const gchar **names = g_variant_get_strv(reply_child, &reply_count);

    size_t offset = strlen(MPRIS_PREFIX);
    for (gsize i = 0; i < reply_count; i += 1) {
        if (g_str_has_prefix(names[i], MPRIS_PREFIX)) {
            // PlayerctlPlayerName *player_name = pctl_player_name_new(
            //     names[i] + offset, pctl_bus_type_to_source(bus_type));
            const char *player_name = names[i] + offset;
            players->push_back(player_name);
            std::cout << "found: " << player_name << std::endl;
        }
    }

    g_object_unref(proxy);
    g_variant_unref(reply);
    g_variant_unref(reply_child);
    g_free(names);

    return 0;
}

// End code from

int send_command_to_player(const std::string &player) {
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
        std::cout << "Error: " << error->message << std::endl;
    } else {
        std::cout << "No error" << std::endl;
    }

    // send play command
    gboolean ret = org_mpris_media_player2_player_call_play_pause_sync(
        playerProxy, NULL, &error);

    if (error) {
        std::cout << "Error: " << error->message << std::endl;
        return -1;
    }

    if (!ret) {
        return -1;
    }

    // Cleanup the proxy
    g_object_unref(playerProxy);
    return 0;
}

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    GError *error;
    error = NULL;

    // List everything that supports the 'org.mpris.MediaPlayer2.player
    // interface
    std::vector<std::string> players;
    int rc = list_player_names_on_bus(&players, G_BUS_TYPE_SESSION, &error);
    if (rc != 0) {
        std::cerr << "Failed to list players rc: " << rc << std::endl;
    }

    for (auto player : players) {
        send_command_to_player(player);
    }
}

