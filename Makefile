# Makefile for lws (lite-webserver)
object = lws_tool

# cross compile
CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc

# compile options
CFLAGS += -Wall -O2
LDFLAGS += -lpthread

# source files
SRCS += lws_tool.c
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
