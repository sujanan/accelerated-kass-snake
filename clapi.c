#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

#include "util.h"

#define KERNEL_SRC "./kernels/sample.cl"

static char *readFile(const char *filename) {
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

// temporary main function
int main(int argc, char **argv) {
    return 0;
}
