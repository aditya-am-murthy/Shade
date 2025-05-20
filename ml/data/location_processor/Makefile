CC = gcc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = -lm

TARGET = location_processor
SRCS = location_processor.c ../../../C_Custom_Files/hashmap.c
OBJS = location_processor.o ../../../C_Custom_Files/hashmap.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 