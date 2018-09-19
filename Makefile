# Makefile for lws (lite-webserver)
object = lws_tool

# cross compile
CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc

# compile options
CFLAGS += -Wall -O2
CFLAGS += -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
CFLAGS += -Itool -Ihttp -Iserver
LDFLAGS += -lpthread

# source files
SRCS += tool/lws_util.c
SRCS += tool/lws_log.c
SRCS += http/lws_http.c
SRCS += http/lws_http_plugin.c 
SRCS += server/lws_socket.c
SRCS += server/lws_tool.c

# object files
OBJS = $(patsubst %.c, %.o, $(SRCS))

.PHONY:all clean

all: $(object)

$(object): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build	"$@

%.o: %.c
	@$(CC) $(CFLAGS) -c $^ -o $@
	@echo "CC	"$@

clean:
	-@rm -f $(OBJS) $(object)
