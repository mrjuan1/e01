#ifndef __MEM_H__
#define __MEM_H__

void *memAlloc(const char *thing, int size);
char *memString(const char *thing, const char *text, ...);

#endif // __MEM_H__
