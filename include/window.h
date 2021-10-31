#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdbool.h>

bool windowInit();
void windowFree();

bool windowRunning();
bool windowActive();
void windowUpdate();

bool windowResized();
int windowWidth(), windowHeight();

void windowResetTicks();
float windowDeltaTime();

void windowQuit();

#endif // __WINDOW_H__
