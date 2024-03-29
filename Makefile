TARGET := fxprog
SRCS := main.c \
		ihex.c

# Compiler used 
CC = gcc	
# Compiler flags:
# -g	adds debugging information to the executable file
# -Wall	turns on most, but not all, compiler warnings
CFLAGS = -g -Wall	
LIBUSB_FLAG = -lusb-1.0

# Define object files
OBJS = $(SRCS:.c=.o)

# build executable 
all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBUSB_FLAG)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)
