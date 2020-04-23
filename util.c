#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

void errorLog(
        const char *file,
        int line, 
        const char *fmt, 
        ...) {
    va_list ap;
    va_start(ap, fmt);
    if (errno)
        fprintf(stderr, "ERROR [%s %d]: (%s)\n", file, line, strerror(errno));
    else
        fprintf(stderr, "ERROR [%s %d]:\n", file, line);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void die(
        const char *file,
        int line, 
        const char *fmt, 
        ...) {
    va_list ap;
    va_start(ap, fmt);
    if (errno)
        fprintf(stderr, "Error [%s %d]: (%s)\n", file, line, strerror(errno));
    else
        fprintf(stderr, "Error [%s %d]:\n", file, line);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(1);
}

char *readFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) 
        DIE("Could not read file: %s", filename);
    fseek(file, 0, SEEK_END);
    int len = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    if (!buf)
        DIE("Memory error");
    fread(buf, 1, len, file);
    buf[len] = '\0';
    return buf;
}

