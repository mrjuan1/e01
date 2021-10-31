#ifndef __LOG_H__
#define __LOG_H__

#define info(msg, ...) logMsg(lt_info, msg, __VA_ARGS__)
#define warn(msg, ...) logMsg(lt_warn, msg, __VA_ARGS__)
#define err(msg, ...) logMsg(lt_err, msg, __VA_ARGS__)

#ifdef DEBUG
#define debug(msg, ...) logMsg(lt_debug, msg, __VA_ARGS__)
#else // DEBUG
#define debug(msg, ...) \
	{ }
#endif // DEBUG

typedef enum { //
	lt_info,
	lt_warn,
	lt_err,
	lt_debug
} logType;

void logMsg(logType type, const char *msg, ...);

#endif // __LOG_H__
