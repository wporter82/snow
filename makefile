CC=gcc
CFLAGS=-Wall
CFLAGS+=-std=c99
LDLIBS=-lncurses
TARGET=snow

all: $(TARGET)

debug: CFLAGS += -DDEBUG -ggdb3
debug: $(TARGET)

$(TARGET): $(TARGET).c

.PHONY : clean
clean:
	$(RM) $(TARGET)

