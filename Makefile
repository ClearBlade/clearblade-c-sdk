CC = gcc

CFLAGS = -g -Wall -fPIC

LDFLAGS = -shared

LIBS = -lpthread  -ljansson -lmosquitto -lcurl -lpaho-mqtt3as

SRC = src

SRCS = $(wildcard $(SRC)/*.c)

OBJS = $(SRCS:.c=.o)

TARGET_LIB = libclearblade.so

all: $(TARGET_LIB)
	@echo Object files created

$(TARGET_LIB): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

clean:
	$(RM) $(TARGET_LIB) $(OBJS) $(SRCS:.c=.d)
