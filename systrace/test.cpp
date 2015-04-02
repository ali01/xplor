#include <cstdio>
#include <cstring>


#define MAX_PATH 256
#define _STR(x) #x
#define STR(x) _STR(x)

char *find_debugfs() {
    static char debugfs[MAX_PATH+1];
    int debugfs_found;
    char type[100];

    std::FILE *fp;

    if ((fp = fopen("/proc/mounts", "r")) == NULL)
        return NULL;

    while (fscanf(fp, "%*s %" STR(MAX_PATH) "s %99s %*s %*d %*d\n", debugfs, type) == 2) {
        if (strcmp(type, "debugfs") == 0)
            break;
    }

    fclose(fp);

    if (strcmp(type, "debugfs") != 0)
        return NULL;

    debugfs_found = 1;

    return debugfs;
}

int main(int argc, char **argv) {
    printf("%s\n", find_debugfs());
}
