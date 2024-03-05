#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <libusb-1.0/libusb.h>

#define FX2LP_VID	0x04b4
#define FX2LP_PID	0x8613

#define CMD_FX2LP_REQUEST	0xA0
#define CPUCS_ADDR			0xE600
#define TIMEOUT				250

libusb_context *ctx = NULL;

int fx2_reset(libusb_device_handle *devHandle, bool reset) {
	int rv = 0;
	uint8_t cpucs = reset;

	rv = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
								 CMD_FX2LP_REQUEST, CPUCS_ADDR, 0/*wIndex*/, &cpucs, sizeof(cpucs), TIMEOUT);
	return rv;
}

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

	dev = libusb_open_device_with_vid_pid(ctx, FX2LP_VID, FX2LP_PID);
	if(dev == NULL) {
		printf("Error, could not find USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
		return -1;
	}
	printf("Found USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);

	printf("Send FX2LP reset command...\n");
	status = fx2_reset(dev, false);
	if(status > 0) {
		printf("FX2LP resetted status:%d\n", status);
	} else {
		printf("Unable to reset FX2LP...\n");
	}

	sleep(2);

	printf("Send FX2LP alive command...\n");
	status = fx2_reset(dev, true);
	if(status > 0) {
		printf("FX2LP alive status:%d\n", status);
	} else {
		printf("Unable to send FX2LP command...\n");
	}

	printf("Closing USB device...\n");
	libusb_close(dev);
	libusb_exit(NULL);
	return 0;
}
