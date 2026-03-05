# Manual install

## Building & Flashing via PlatformIO

0. Install the following tools:
    - [Visual Studio Code](https://code.visualstudio.com/) and install the PlatformIO IDE plugin. 
    - [Git](https://git-scm.com/download/win)
1. Download the source code of CYD-Klipper
    - This can be done via the `git clone https://github.com/suchmememanyskill/CYD-Klipper` command or via the green `<> Code` button on Github
2. Open the CYD-Klipper folder inside the CYD-Klipper folder in Visual Studio Code
3. Click on the Alien/Bug tab (PlatformIO) on the left
4. Expand the folder/tab for your specific screen
    - Entries with the suffix '-SD' are using the smartdisplay driver. Entries without this suffix are using a custom driver
    - Usually, a custom driver is preferred over the smartdisplay driver
5. Connect the display to your computer
6. Click 'Upload and Monitor'
    - This will start compiling the code, and after upload it to the display
    - Don't forget to hold the boot button while flashing. The screen will flash when the firmware is attempted to be flashed
