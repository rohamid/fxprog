# Define source files
SRCS := main.c \
		ihex.c \
		usbdev.c \
		usb_fx2.c \
		usb_benchmark.c

# Compiler used
CC = gcc
# Compiler flags:
# -g	adds debugging information to the executable file
# -Wall	turns on most, but not all, compiler warnings
CFLAGS = -g -Wall
LIBUSB_FLAG = -lusb-1.0

# Define object file directory
OBJDIR = build

# Define object files
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))

# Define the executable
TARGET = $(OBJDIR)/fxprog


# build executable 
all: $(TARGET)

# Rule to create the build directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Rule to link the object files and create the executable
$(TARGET): $(OBJS) | $(OBJDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBUSB_FLAG)

# Rule to compile .c files into .o files
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf $(OBJDIR)
