#include "System.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string.h>
#include <io.h>

#include <direct.h>
#include <stdlib.h>


/* 获得指定目录下的所有文件名 */
int System_GetFiles(char *files[], int n, const char *dir)
{
    struct _finddata_t file;
    char buf[256];
    long handle;
    int i, total = 0;
    char *path = NULL, *path_bak = _getcwd(NULL, 0);

    strcpy(buf, dir);
    for (i = strlen(buf); i >= 0 && buf[i] != '\\' && buf[i] != '/'; i--);
    if (i >= 0)
    {
        buf[i] = 0;
        if (_chdir(buf) < 0) {
            free(path_bak);
            return 0;
        }
    }  
    path = _getcwd(NULL, 0);
    _chdir(path_bak);
    free(path_bak);

    handle = _findfirst(dir, &file);
    if (handle == -1) {
        free(path);
        return 0;
    }

    do {
        files[total] = (char *)malloc(strlen(file.name) + strlen(path) + 1);
        if (files[total] != NULL && file.name[0] != '.') {
            strcpy(files[total], path);
            strcat(files[total], "\\");
            strcat(files[total++], file.name);
        }
    } while (total < n && !_findnext(handle, &file));

    free(path);
    return total;
}
