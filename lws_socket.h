#ifndef _LWS_SOCKET_H_
#define _LWS_SOCKET_H_

/**
 * @func    lws_set_socket_reuse
 * @brief   set socket reuse attribution
 *
 * @param   sockfd[in] local socket fd
 * @return  On success, return 0, On error, return error code.
 */
extern int lws_set_socket_reuse(int sockfd);

/**
 * @func    lws_accept_handler
 * @brief   recv remote socket data
 *
 * @param   sockfd[in] local socket fd
 * @return  On success, return 0, On error, return error code.
 */
extern int lws_accept_handler(int sockfd);

/**
 * @func    lws_service_start
 * @brief   start lite-web-server service
 *
 * @param   port[in] bind local port
 * @return   On success, return 0, On error, return error code.
 */
extern int lws_service_start(short port);

#endif // _LWS_SOCKET_H_

