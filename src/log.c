#include "log.h"

#include <stdarg.h>

#ifdef LOG_USE_SDL
#include <SDL2/SDL_log.h>
#endif // LOG_USE_SDL

void logMsg(logType type, const char *msg, ...) {
#ifndef DEBUG
	if(type == lt_debug) return;
#endif // DEBUG

	va_list list;
	va_start(list, msg);

#ifdef LOG_USE_SDL
	SDL_LogPriority priority;

	switch(type) {
		case lt_info: priority = SDL_LOG_PRIORITY_INFO; break;
		case lt_warn: priority = SDL_LOG_PRIORITY_WARN; break;
		case lt_err: priority = SDL_LOG_PRIORITY_ERROR;
#ifdef DEBUG
			break;
		case lt_debug: priority = SDL_LOG_PRIORITY_DEBUG;
#endif // DEBUG
	}

	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, priority, msg, list);
#endif // LOG_USE_SDL

	va_end(list);
}
