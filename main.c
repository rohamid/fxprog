#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#define FX2LP_VID	0x04b4
#define FX2LP_PID	0x8613

libusb_context *ctx = NULL;

int usb_write(libusb_device_handle *dev, const char *label,
			 uint8_t opcode, uint32_t address, const uint8_t *data, uint8_t len) {
	

	return 0;
}

int main(int argc, char *argv[]) {
	int status;
	libusb_device_handle *dev = NULL;

	status = libusb_init(&ctx);
	if(status != 0) {
		printf("Error : %s\n", libusb_error_name(status));
		return -status;
	}

	dev = libusb_open_device_with_vid_pid(NULL, FX2LP_VID, FX2LP_PID);
	if(dev == NULL) {
		printf("Error, could not find USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
//		perror("Error, could not find USB device ");	
//		printf("Error: %s\n", libusb_error_name(dev));
		return -1;
	}
	printf("Found USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
	printf("Closing USB device...\n");
	libusb_close(dev);
	libusb_exit(NULL);
	return 0;
}
