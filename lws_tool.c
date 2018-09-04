#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "lws_log.h"
#include "lws_socket.h"

void print_usage(void)
{
    printf("Usage: lws_tool [options...]\n");
    printf("Options:\n");
    printf("    -s start  local service\n");
    printf("    -p port  select local port, default is 8000\n");
    printf("    -l level  set syslog level, 0-all,1-sys,2-error,3-warning,4-info\n");
    printf("              default log level is 3-warning\n");
    printf("    -h  print usage information\n");
}

/**
 * @func    main
 * @brief   lws_tool execulate binary
 *
 * @param   argc, argv - cammand line input
 * @return   On success, exit 0, On error, exit error code.
 */
int main(int argc, char *argv[])
{
    int port = 8000;
    int service = 0;
    log_level_t log_level = LOG_LEVEL_WARN;
    char ch;

    if (argc < 2) {
        lws_log(3, "argc: %d\n", argc);
        goto usage;
    }

    while ((ch = getopt(argc, argv, "sp:l:h")) != -1) {
        switch (ch) {
            case 's':
                service = 1;
                break;

            case 'p':
                port = atoi(optarg);
                break;

            case 'l':
                log_level = atoi(optarg);
                break;

            case 'h':
                goto usage;

            case '?':
            default:
                lws_log(2, "Unknow option\n");
                break;
        }
    }

    if (log_level < LOG_LEVLE_SYS || log_level > LOG_LEVEL_INFO) {
        lws_log(2, "log level input error, level: %d\n", log_level);
        goto usage;
    }

    /* set syslog level */
	lws_set_log_level(log_level);

    /* start lws service */
    if (service) {
        lws_log(3, "start lws service, port: %d\n", port);
        lws_service_start(port);
    }

	return 0;

usage:
    print_usage();
    return -1;
}

