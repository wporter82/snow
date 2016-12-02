CC=gcc
# Debug
# CFLAGS=-Wall -g -O0 -DDEBUG
# Release
CFLAGS=-Wall -O2 -DNDEBUG
LIBS=-lncursesw
TARGET=snow

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(TARGET).c $(CFLAGS) $(LIBS) 

.PHONY : clean
clean:
	$(RM) $(TARGET)

