#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "lws_log.h"
#include "lws_util.h"

/**
 * @func    lws_ftell_file
 * @brief   get file size
 *
 * @param   filename[in] input file name
 * @return  On success, return file size. On error, return -1.
 **/
long lws_ftell_file(char *filename)
{
    FILE *fp = NULL;
    long offset;

    if (filename == NULL)
        return -1;

    if (access(filename, F_OK))
        return -1;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        lws_log(2, "fopen failed %s\n", strerror(errno));
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    offset = ftell(fp);
    fclose(fp);

    return offset;
}

/**
 * @func    lws_basename
 * @brief   get base filename /home/cfg/settings.json -> settings.json
 *
 * @param   filename[in] input filename
 * @return  On success, return basename. Or return input filename.
 **/
char *lws_basename(char *filename)
{
    char *last = NULL, *cur = NULL;

    cur = filename;
    while (cur != NULL && *cur != '\0') {
        if (*cur == '/') {
            last = cur + 1;
        }
        cur++;
    }

    if (last == NULL && *cur == '\0') {
        return filename;
    }

    return last;
}

/**
 * @func    lws_read_file
 * @brief   read file data
 *
 * @param   filename[in] input file name
 * @param   data[out] file data
 * @param   size[int] read length
 * @return  On success, return read size. On error, return -1.
 **/
int lws_read_file(char *filename, void *data, int size)
{
    FILE *fp = NULL;
    int rlen = 0;

    if (filename == NULL || data == NULL || size <= 0)
        return -1;

    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    rlen = fread(data, 1, size, fp);
    fclose(fp);

    return rlen;
}


