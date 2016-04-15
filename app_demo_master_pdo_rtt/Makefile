CC = gcc
LD = gcc

INCLUDE = -I/opt/etherlab/include
LIB_PATH = -L/opt/etherlab/lib
LIBRARY = -lethercat

CFLAGS = -Wall --std=gnu99 -g -O2 $(INCLUDE) 
LDFLAGS = -static -Wall -Wextra $(LIB_PATH) $(LIBRARY)

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = test_master

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *~ *.o $(TARGET)
