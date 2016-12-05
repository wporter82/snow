CC=gcc
CFLAGS=-Wall
LIBS=-lncursesw
TARGET=snow

all: $(TARGET)

debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(TARGET).c $(CFLAGS) $(LIBS) 

.PHONY : clean
clean:
	$(RM) $(TARGET)

