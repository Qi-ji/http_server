#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h> 

#include "lws_log.h"

/* print information buflen */
#define LWS_LOG_MAX_LEN  2048

static log_level_t sys_log_level = LOG_LEVEL_WARN;

void lws_set_log_level(log_level_t level)
{
    sys_log_level = level;
}

log_level_t lws_get_log_level(void)
{
    return sys_log_level;
}

/**
 * @func    lws_basename
 * @brief   get base filename /home/uesr/settings.txt -> settings.txt
 *
 * @param   filename[in] input filename
 * @return  On success, return basename. Or return input filename.
 **/
static char *lws_basename(char *filename)
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
 * @func    log_level_info
 * @brief   Get the name of the level
 *
 * @param   level [IN] int type
 *
 * @return   On success, return level name. On Error, return all : the level name.
 */
char *lws_log_level_info(int level)
{
    char *info[] = {"ALL", "SYS", "ERR", "WARN", "DBG"};

    if (level < 5)
        return info[level];

    return info[0];
}

/**
 * @func    lws_logger
 * @brief   output print information interface
 *
 * @param   level[in] debug print level
 * @param   filename[in] debug information filename
 * @param   line[in] debug information line
 * @param   format[in] debug information format
 * @return  print message size.
 */
int lws_logger(log_level_t level, const char *filename, int line, const char *format, ...)
{
    char logmsg[LWS_LOG_MAX_LEN];
    struct tm stru_curtime;
    time_t curtime;
    va_list ap;
    int msglen = 0;
    char *base_filename;

    if ((NULL == filename) || (NULL == format)) {
        return 0;
    }

    if (level > sys_log_level) {
        return 0;
    }

    memset(logmsg, 0, LWS_LOG_MAX_LEN);
    memset(&ap, 0, sizeof(va_list));

    base_filename = lws_basename((char *)filename);

    curtime = time(NULL);
    (void)localtime_r(&curtime, &stru_curtime);

    va_start(ap, format);
    msglen = sprintf(logmsg, "[%02d-%02d %02d:%02d:%02d][%s][%s:%d] ",
            stru_curtime.tm_mon + 1, stru_curtime.tm_mday,
            stru_curtime.tm_hour, stru_curtime.tm_min, stru_curtime.tm_sec,
            lws_log_level_info(level),
            base_filename ? base_filename : "UNKOWN FILE",
            line);
    vsnprintf(&logmsg[msglen], LWS_LOG_MAX_LEN - msglen, format, ap);
    logmsg[LWS_LOG_MAX_LEN - 2] = '\n';
    logmsg[LWS_LOG_MAX_LEN - 1] = '\0';
    va_end(ap);

    if (level <= sys_log_level) {
        printf("%s", logmsg);
    }

    return msglen;
}

