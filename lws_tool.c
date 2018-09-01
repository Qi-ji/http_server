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

typedef enum {
	LOG_LEVLE_SYS = 1,
	LOG_LEVEL_ERR = 2,
	LOG_LEVEL_WARN = 3,
	LOG_LEVEL_INFO = 4
} log_level_t;

static int sys_log_level = 2;

#define lws_log(level, ...) do { \
				if (level <= sys_log_level) { \
					printf("[%s][%d]", __FILE__, __LINE__); \
					printf(__VA_ARGS__); \
				} \
			} while (0);

static void lws_set_log_level(int level)
{
	sys_log_level = level;
}

int lws_accept_handler(void *arg)
{
	int cli_fd = (int)arg;
	int nread = 0;
	fd_set rset;
	char pread_buf[4096];
	struct timeval select_timeout;
	int ret;

	if (arg == NULL || cli_fd < 0) {
		lws_log(2, "arg error\n");
		return -1;
	}

	while (1) {
		select_timeout.tv_sec = 10;
		select_timeout.tv_usec = 0;

		FD_ZERO(&rset);
		FD_SET(cli_fd, &rset);
		ret = select(cli_fd + 1, &rset, NULL, NULL, &select_timeout);
		if (ret < 0) {
			lws_log(2, "select failed, fd: %d, err: %s\n", cli_fd, strerror(errno));
			break;
		} else if (ret == 0) {
			lws_log(4, "select timeout\n");
			continue;
		}

		if (FD_ISSET(cli_fd, &rset)) {
			memset(pread_buf, 0, 4096);
			nread = recv(cli_fd, pread_buf, 4096, 0);
			if (nread < 0) {
			    perror("recv");
			    if (EINTR == errno) {
			        nread = 0;
			    } else {
			        printf("recv, %s\n", strerror(errno));
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

	close(cli_fd);
	return 0;
}

static int lws_start(short port)
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
		ret = pthread_create(&tid, NULL, (void *)lws_accept_handler, (void *)cli_fd);
		if (ret) {
			lws_log(2, "create thread failed, ret: %d-%s\n", ret, strerror(errno));
			continue;
		}
	}

	close(sockfd);
    return 0;
}

int main(int argc, char *argv[])
{
	lws_set_log_level(4);

    lws_start(8000);

	return 0;
}
