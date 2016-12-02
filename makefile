CC=gcc
# Debug
# CFLAGS=-Wall -g -O0 -DDEBUG
# Release
CFLAGS=-Wall -O2 -DNDEBUG
LIBS=-lncursesw
TARGET=snow

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(LIBS) -o $(TARGET) $(TARGET).c

.PHONY : clean
clean:
	$(RM) $(TARGET)

