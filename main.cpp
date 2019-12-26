#include <dbus/dbus-glib.h>  // for dbus_g_*
#include <dbus/dbus.h>       // for dbus_*
#include <generated_dbus.h>
#include <generated_mediaplayer2.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <iostream>

#define UNUSED(expr)  \
    do {              \
        (void)(expr); \
    } while (0)

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    GError *error;
    error = NULL;

    // List everything that supports the 'org.mpris.MediaPlayer2.player
    // interface
    OrgFreedesktopDBus *dbusProxy;
    dbusProxy =
        org_freedesktop_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    "org.freedesktop.DBus",
                                                    "/org/freedesktop/DBus",
                                                    NULL,
                                                    &error);

    if (error) {
        std::cout << "Error: " << error->message << std::endl;
    } else {
        std::cout << "No error" << std::endl;
    }

    // Create the proxy
    OrgMprisMediaPlayer2Player *playerProxy;
    playerProxy = org_mpris_media_player2_player_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION,
        G_DBUS_PROXY_FLAGS_NONE,
        "org.mpris.MediaPlayer2.spotify",
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
    } else {
        std::cout << "No error" << std::endl;
    }

    if (ret) {
        std::cout << "ret is true" << std::endl;
    } else {
        std::cout << "ret is false" << std::endl;
    }

    // Cleanup the proxy
    g_object_unref(playerProxy);
}

