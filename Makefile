CC = gcc

CFLAGS = -g -Wall -fPIC

LDFLAGS = -shared

LIBS = -lpthread  -ljansson -lcurl -lpaho-mqtt3as

SRC = src

SRCS = $(wildcard $(SRC)/*.c)

OBJS = $(SRCS:.c=.o)

TARGET_LIB = libclearblade.so

PREFIX = /usr/local

all: $(TARGET_LIB)
	@echo Object files created

$(TARGET_LIB): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(TARGET_LIB) $(OBJS) $(SRCS:.c=.d)

install: $(TARGET_LIB)
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	cp $(TARGET_LIB) $(PREFIX)/lib/$(TARGET_LIB)
	cp $(SRC)/*.h $(PREFIX)/include/
	ldconfig $(PREFIX)/lib
	rm $(TARGET_LIB)
