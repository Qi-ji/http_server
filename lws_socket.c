#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "lws_log.h"

/**
 * @func    lws_set_socket_reuse
 * @brief   set socket reuse attribution
 *
 * @param   sockfd[in] local socket fd
 * @return  On success, return 0, On error, return error code.
 */
int lws_set_socket_reuse(int sockfd)
{
    int opt = 1;
    int ret;

    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(opt));
    if (ret) {
        lws_log(2, "setsockopt reuse failed, %s\n", strerror(errno));
        return -1;
    }

    lws_log(4, "set socket reuse success\n");
    return 0;
}

/**
 * @func    lws_accept_handler
 * @brief   recv remote socket data
 *
 * @param   sockfd[in] local socket fd
 * @return  On success, return 0, On error, return error code.
 */
int lws_accept_handler(int sockfd)
{
	int nread = 0;
	fd_set rset;
	char pread_buf[4096];
	struct timeval select_timeout;
	int ret;

	if (sockfd < 0) {
		lws_log(2, "arg error\n");
		return -1;
	}

	while (1) {
		select_timeout.tv_sec = 10;
		select_timeout.tv_usec = 0;

		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		ret = select(sockfd + 1, &rset, NULL, NULL, &select_timeout);
		if (ret < 0) {
			lws_log(2, "select failed, fd: %d, err: %s\n", sockfd, strerror(errno));
			break;
		} else if (ret == 0) {
			lws_log(4, "select timeout\n");
			continue;
		}

		if (FD_ISSET(sockfd, &rset)) {
			memset(pread_buf, 0, 4096);
			nread = recv(sockfd, pread_buf, 4096, 0);
			if (nread < 0) {
			    perror("recv");
			    if (EINTR == errno) {
			        nread = 0;
			    } else {
			        lws_log(4, "recv, %s\n", strerror(errno));
			        break;
			    }
			} else if (0 == nread) {
			    break;
			} else {
				lws_log(4, "recv: %s\n", pread_buf);
				break;
			}
		}
	}

	close(sockfd);
	return 0;
}

static void *lws_accept_thread(void *arg)
{
    int sockfd = (int)arg;
    int ret;

    if (arg == NULL) {
        lws_log(2, "lws accept thread arg invalid\n");
        return NULL;
    }

    ret = lws_accept_handler(sockfd);
    if (ret) {
        lws_log(2, "accept handler failed, ret: %d\n", ret);
        return NULL;
    }

    return NULL;
}

/**
 * @func    lws_service_start
 * @brief   start lite-web-server service
 *
 * @param   port[in] bind local port
 * @return  On success, return 0, On error, return error code.
 */
int lws_service_start(short port)
{
    int sockfd, cli_fd;
	struct sockaddr_in sockaddr;
	struct sockaddr_in cli_addr;
	unsigned int cli_addrlen = 0;
	pthread_t tid;
	int ret;

    /* create local socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
		lws_log(2, "socket failed: %s\n", strerror(errno));
		return -1;
	}

	lws_log(4, "socket success, fd: %d\n", sockfd);

    /* socket attribution before start accept */
	lws_set_socket_reuse(sockfd);

    /* bind local port */
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	if (ret) {
		lws_log(2, "bind failed, ret: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

    lws_log(4, "bind success, start listen\n");

    /* set listen client count */
	ret = listen(sockfd, 128);
	if (ret) {
		lws_log(2, "listen failed, ret: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	lws_log(4, "listen succes, start accept\n");

	while (1) {
	    /* start accept linkage */
		cli_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_addrlen);
		if (cli_fd < 0) {
			lws_log(2, "accept failed, ret: %s\n", strerror(errno));
			continue;
		}

        /* create thread to handle clinet message */
		ret = pthread_create(&tid, NULL, (void *)lws_accept_thread, (void *)cli_fd);
		if (ret) {
			lws_log(2, "create thread failed, ret: %d-%s\n", ret, strerror(errno));
			close(cli_fd);
			continue;
		}
	}

	close(sockfd);
    return 0;
}

