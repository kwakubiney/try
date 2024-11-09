#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>

int link(const char *oldpath, const char *newpath) {
    // Get the original `link` function
    int (*orig_link)(const char*, const char*) = dlsym(RTLD_NEXT, "link");

    // Try the original link function
    int ret = orig_link(oldpath, newpath);
    if (ret == 0 || errno != EXDEV) {
        return ret; // Return if successful or if an error other than EXDEV
    }

    // Handle EXDEV by copying and removing the original file
    FILE *src = fopen(oldpath, "rb");
    if (!src) return -1;

    FILE *dest = fopen(newpath, "wb");
    if (!dest) {
        fclose(src);
        return -1;
    }

    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, n, dest) != n) {
            fclose(src);
            fclose(dest);
            return -1;
        }
    }

    fclose(src);
    fclose(dest);

    // Optionally, unlink the original file (to mimic a move)
    return unlink(oldpath);
}