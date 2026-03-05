#include "serial_commands.h"
#include <HardwareSerial.h>
#include <Esp.h>
#include <cstring>
#include "../../conf/global_config.h"
#include "../../core/printer_integration.hpp"

namespace serial_console {

/* How to add a new command:
  - add the handler; function signature must be like: void handler(String argv[])
  - add {"command_name", &handler_name, argc} to commandHandlers
        (argc = num of args + 1; argv[0] is always the command name)
  - add the handler signature to serial_command.h
  - add description to help()
  - optionally add handling the new preference to sets() and settings() if it modifies global_config
*/

PrinterConfiguration* get_current_printer_config()
{
    return &global_config.printer_config[global_config.printer_index];
}

HANDLER commandHandlers[] = {
    {"help", &help, 1},
    {"reset", &reset, 1},
    {"settings", &settings, 1},
    {"sets", &sets, 1},
    {"erase", &erase, 2},
    {"key", &key, 2},
    {"touch", &touch, 5},
    {"ssid", &ssid, 3},
    {"ip", &ip, 3},
    {"rotation", &rotation, 2},
    {"brightness", &brightness, 2},
    {"printer", &printer, 2},
    {"debug", &debug, 2},
    {"echo", &echo, 2}
};

void help(String argv[])
{
    Serial1.println("Serial console commands:");
    Serial1.println("");
    Serial1.println("settings             - show current settings");
    Serial1.println("sets                 - show current settings as commands for copy-paste");
    Serial1.println("erase [item]         - unconfigure parameter (key|touch|ssid|ip|all)");
    Serial1.println("reset                - restart CYD-klipper");
    Serial1.println("touch [xm xo ym yo]  - set touchscreen multipliers and offsets");
    Serial1.println("ssid [name pass]     - set the network SSID and password to connect to");
    Serial1.println("ip [address port]    - set Moonraker address");
    Serial1.println("key [key]            - set the Moonraker API key");
    Serial1.println("rotation [on|off]    - set rotate screen 180 degrees");
    Serial1.println("brightness [num]     - set screen brightness");
    Serial1.println("printer [num|-1]     - set active printer#; -1 for multi-printer mode off");
    Serial1.println("debug [on|off]       - set printing of debug messages to serial console (not saved)");
    Serial1.println("echo [on|off]        - set remote echo (eecchhoo ooffff) (not saved)");
    Serial1.println("help                 - this help");
    Serial1.println("");
    Serial1.println("Settings are saved immediately but come into effect after reset");
    Serial1.println("Unlike GUI, serial console does not validate if settings");
    Serial1.println("you enter work correctly. This is a double-edged sword.");
}

// this must be here, because serial_console doesn't have a clue about sizeof(commandHandlers) at compile time
int find_command(String cmd)
{
  for(int i=0; i < sizeof(commandHandlers) / sizeof(HANDLER); ++i)
  {
    if(cmd == commandHandlers[i].name) return i;
  }
  Serial1.println("Unknown command");
  return -1;
}


void reset(String argv[])
{
    ESP.restart();
}

void sets(String argv[])
{

    Serial1.printf("printer %d\n", global_config.multi_printer_mode?global_config.printer_index:-1);

    if(global_config.wifi_configured)
    {
        Serial1.printf("ssid %s %s\n",global_config.wifi_SSID,
#if DISPLAY_SECRETS
        global_config.wifi_password
#else
        "[redacted]"
#endif
        );
    }
    else
    {
        Serial1.printf("erase ssid\n");
    }

    if(get_current_printer_config()->ip_configured)
    {
        Serial1.printf("ip %s %d\n",get_current_printer_config()->printer_host, get_current_printer_config()->klipper_port);
    }
    else
    {
        Serial1.printf("erase ip\n");
    }

    if(get_current_printer_config()->auth_configured)
    {
        Serial1.printf("key %s\n",
#if DISPLAY_SECRETS
        get_current_printer_config()->printer_auth
#else
        "[redacted]"
#endif
        );
    }
    else
    {
        Serial1.printf("erase key\n");
    }

    if(global_config.screen_calibrated)
    {
        Serial1.printf("touch %f %f %f %f\n",
        global_config.screen_cal_x_mult, global_config.screen_cal_x_offset, global_config.screen_cal_y_mult, global_config.screen_cal_y_offset);
    }
    else
    {
        Serial1.printf("erase touch\n");
    }

    Serial1.printf("rotation %s\n",global_config.rotate_screen?"on":"off");
    Serial1.printf("brightness %d\n",global_config.brightness);
}

void settings(String argv[])
{

    if(get_current_printer_config()->printer_name[0] != 0)
    {
        Serial1.printf("Current printer# %d name: %s",global_config.printer_index, get_current_printer_config()->printer_name);
    }
    else
    {
        Serial1.printf("Current printer# %d",global_config.printer_index);
    }
    Serial1.printf("  Multi-printer mode %s\n",global_config.multi_printer_mode?"enabled":"disabled");


    if(global_config.wifi_configured)
    {
        Serial1.printf("SSID: %s   Password: %s\n",global_config.wifi_SSID,
#if DISPLAY_SECRETS
        global_config.wifi_password
#else
        "[redacted]"
#endif

        );
    }
    else
    {
        Serial1.printf("Wifi not configured\n");
    }

    if(get_current_printer_config()->ip_configured)
    {
        Serial1.printf("Moonraker address: %s:%d\n",get_current_printer_config()->printer_host, get_current_printer_config()->klipper_port);
    }
    else
    {
        Serial1.printf("Moonraker address not configured\n");
    }

    if(get_current_printer_config()->auth_configured)
    {
        Serial1.printf("Moonraker API key: %s\n",
#if DISPLAY_SECRETS
        get_current_printer_config()->printer_auth
#else
        "[redacted]"
#endif
        );
    }
    else
    {
        Serial1.printf("Moonraker API key not configured\n");
    }

    if(global_config.screen_calibrated)
    {
        Serial1.printf("Screen coefficients: x_screen = %f * x_touch + %f;  y_screen = %f * y_touch + %f\n",
        global_config.screen_cal_x_mult, global_config.screen_cal_x_offset, global_config.screen_cal_y_mult, global_config.screen_cal_y_offset);
    }
    else
    {
        Serial1.printf("Screen not calibrated\n");
    }

    Serial1.printf("Screen orientation: %s\n",global_config.rotate_screen?"rotated":"normal");
    Serial1.printf("Screen brightness: %d\n",global_config.brightness);
}


void erase_one(const String arg)
{
    if(arg == "key")
    {
        get_current_printer_config()->auth_configured = false;
        // overwrite the key to make it unrecoverable for 3rd parties
        memset(get_current_printer_config()->printer_auth,0,64);
        write_global_config();
    }
    else if(arg == "ip")
    {
        get_current_printer_config()->setup_complete = false;
        get_current_printer_config()->ip_configured = false;
        write_global_config();
    }
    else if(arg == "touch")
    {
        global_config.screen_calibrated = false;
        write_global_config();
    }
    else if(arg == "ssid")
    {
        global_config.wifi_configured = false;
        // overwrite the pass to make it unrecoverable for 3rd parties
        memset(global_config.wifi_password,0,64);
        write_global_config();
    }
    else
    {
        Serial1.println("Unknown key");
    }
}

void erase(String argv[])
{
    const String& arg=argv[1];
    if(arg != "all")
    {
        erase_one(arg);
    }
    else
    {
        erase_one("key");
        erase_one("ip");
        erase_one("touch");
        erase_one("ssid");
    }
}

void key(String argv[])
{
    get_current_printer_config()->auth_configured = true;
    strncpy(get_current_printer_config()->printer_auth, argv[1].c_str(), sizeof(global_config.printer_config[0].printer_auth));
    write_global_config();
}

void touch(String argv[])
{
    global_config.screen_cal_x_mult = argv[1].toFloat();
    global_config.screen_cal_x_offset = argv[2].toFloat();
    global_config.screen_cal_y_mult = argv[3].toFloat();
    global_config.screen_cal_y_offset = argv[4].toFloat();
    global_config.screen_calibrated = true;
    write_global_config();
}

void ssid(String argv[])
{
    strncpy(global_config.wifi_SSID, argv[1].c_str(), sizeof(global_config.wifi_SSID)-1);
    strncpy(global_config.wifi_password, argv[2].c_str(), sizeof(global_config.wifi_password)-1);
    global_config.wifi_configured = true;
    write_global_config();
}

void ip(String argv[])
{
    strncpy(get_current_printer_config()->printer_host, argv[1].c_str(), sizeof(global_config.printer_config[0].printer_host)-1);
    get_current_printer_config()->klipper_port =  argv[2].toInt();
    get_current_printer_config()->ip_configured = true;
    get_current_printer_config()->setup_complete = true;
    write_global_config();
}

void rotation(String argv[])
{
    if(argv[1] == "on")
    {
        global_config.rotate_screen = true;
        write_global_config();
    }
    else if (argv[1] == "off")
    {
        global_config.rotate_screen = false;
        write_global_config();
    }
    else
    {
        Serial1.println("Rotation can be on or off");
    }
}

void brightness(String argv[])
{
    global_config.brightness = argv[1].toInt();
    write_global_config();
}


void printer(String argv[])
{
    int ndx = argv[1].toInt();
    if(ndx <= -1)
    {
        global_config.multi_printer_mode = false;
        set_current_printer(0);
    }
    else if( ndx >= 0 && ndx < get_printer_count())
    {
        global_config.multi_printer_mode = true;
        set_current_printer(ndx);
    }
    else
    {
        Serial1.println("Printer index out of range");
    }

}

void debug(String argv[])
{
    if(argv[1] == "on")
    {
        temporary_config.debug = true;

    }
    else if (argv[1] == "off")
    {
        temporary_config.debug = false;
    }
    else
    {
        Serial1.println("debug can be on or off");
    }
}

void echo(String argv[])
{
    if(argv[1] == "on")
    {
        temporary_config.remote_echo = true;
    }
    else if (argv[1] == "off")
    {
        temporary_config.remote_echo = false;
    }
    else
    {
        Serial1.println("Echo can be on or off");
    }
}


}