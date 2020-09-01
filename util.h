#ifndef UTIL_H
#define UTIL_H

void errorLog(const char *file, int line, const char *fmt, ...);
#define ERROR_LOG(...) errorLog(__FILE__, __LINE__, __VA_ARGS__)

void die(const char *file, int line, const char *fmt, ...);
#define DIE(...) die(__FILE__, __LINE__, __VA_ARGS__)

char *readFile(const char *filename);

#endif
