#include <stdio.h>
#include <string.h>

#include "lws_log.h"
#include "lws_socket.h"

/**
 * @func    main
 * @brief   lws_tool execulate binary
 *
 * @param   argc, argv - cammand line input
 * @return   On success, exit 0, On error, exit error code.
 */
int main(int argc, char *argv[])
{
	lws_set_log_level(4);

    lws_service_start(8000);

	return 0;
}
