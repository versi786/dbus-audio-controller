#include <iostream>
#include <glib.h>
#include <glib/gprintf.h>
#include <dbus/dbus.h> // for dbus_*
#include <dbus/dbus-glib.h> // for dbus_g_*
#include <gio/gio.h>
#include <generated_mediaplayer2.h>

int main(int argc, char **argv) {
    gchar c = 'a';
    std::cout << "Hello World\n" << c << std::endl;

    OrgMprisMediaPlayer2Player *proxy;
    GError *error;
    error = NULL;

    // Create the proxy
    proxy = org_mpris_media_player2_player_proxy_new_for_bus_sync (
            G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_NONE,
            "org.mpris.MediaPlayer2.spotify",
            "/org/mpris/MediaPlayer2",
            NULL,
            &error);

    if(error) {
        std::cout << error << std::endl;
    } else {
        std::cout << "No error" << std::endl;
    }

    // send play command
    gboolean ret = org_mpris_media_player2_player_call_play_sync (
    proxy,
    NULL,
    &error);

    if(error) {
        std::cout << error << std::endl;
    } else {
        std::cout << "No error" << std::endl;
    }

    if (ret) {
        std::cout << "ret is true" << std::endl;
    } else {
        std::cout << "ret is false" << std::endl;
    }


    // Cleanup the proxy
    g_object_unref(proxy);
}



