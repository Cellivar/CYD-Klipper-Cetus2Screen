#pragma once
// Adapted from https://github.com/xperiments-in/xtouch/blob/main/src/devices/2.8/screen.h

#ifdef HAS_BUZZER
#include <ezBuzzer.h>
extern ezBuzzer buzzer;
#endif

void screen_setBrightness(unsigned char brightness);
void screen_setup();
void set_invert_display();