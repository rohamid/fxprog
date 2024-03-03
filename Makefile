TARGET := fxprog
SRCS := main.c

# Compiler used 
CC = gcc	
# Compiler flags:
# -g	adds debugging information to the executable file
# -Wall	turns on most, but not all, compiler warnings
CFLAGS = -g -Wall	
LIBUSB_FLAG = -lusb-1.0

# build executable 
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBUSB_FLAG)

.PHONY: clean

clean:
	$(RM) $(TARGET)
