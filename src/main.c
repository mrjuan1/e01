#include <signal.h>

#include "config.h"
#include "e01.h"
#include "log.h"
#include "renderer.h"
#include "texture.h"
#include "window.h"

#define fail()        \
	{                 \
		exitCode = 1; \
		goto end;     \
	}

void signalHandler(int signal);

int main() {
	int exitCode = 0;

	signal(SIGINT, signalHandler);

	if(!configInit() || !windowInit()) fail();

	rendererSystemInit();
	textureSystemInit();

	if(!e01Init()) fail();

	// debug("Starting main loop...\n", 0);
	windowResetTicks();

	while(windowRunning()) {
		if(windowActive()) {
			if(windowResized()) e01Resize();
			e01Run();
		}

		windowUpdate();
	}

end:

	// debug("Quitting...\n", 0);

	e01Free();
	windowFree();
	configFree();

	return exitCode;
}

void signalHandler(int signal) {
	if(signal == SIGINT) {
		debug("Interrupt signal received.\n", 0);
		windowQuit();
	}
}
