#include "usbdev.h"

void usb_error_check(int error_check, const char *str) {
	if(error_check < 0) {
		printf("%s: %s\n", str, libusb_error_name(error_check));
		exit(1);
	}
}

bool fx2IsOpen(libusb_device_handle *usbHandle) {
	return true;
}

/*
 * @brief init libusb using defined context
 */
void usb_init(libusb_context **ctx) {
	int rc = libusb_init(ctx);

	usb_error_check(rc, "libusb_init()");
}

bool usb_open(libusb_device_handle **devHandle, libusb_context **ctx, int vid, int pid) {
	libusb_device **devs;
	ssize_t cnt;

	// Get device list of USB devices
	cnt = libusb_get_device_list(*ctx, &devs);
	if(cnt < 0) {
		fprintf(stderr, "Error getting device list\n");
		libusb_exit(*ctx);
		return -1;
	}

	// Iterate through devices
	for(ssize_t i = 0; i < cnt; i++) {
		libusb_device *dev = devs[i];
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if(r < 0) {
			fprintf(stderr, "Error getting device list\n");
			continue;
		}

		// Check VID and PID
		if(desc.idVendor == vid && desc.idProduct == pid) {
			// Open device
			if(libusb_open(dev, devHandle) == 0) {
				// Set auto-detach kernel driver option
				if(libusb_set_auto_detach_kernel_driver(*devHandle, 1) != 0) {
					fprintf(stderr, "Error setting auto-detach kernel driver option\n");
				}
				libusb_free_device_list(devs, 1); // Free device list
				return true; 	// Success
			} else {
				fprintf(stderr, "Error opening USB device\n");
				libusb_free_device_list(devs, 1); // Free device list
				libusb_exit(*ctx);	// Exit libusb
				return false;
			}
		}
	}

	// Device not found
	fprintf(stderr, "USB device not found\n");
	libusb_free_device_list(devs, 1);	// Free device list
	libusb_exit(*ctx);	// Exit libusb
	return false;
}

void usb_close(libusb_device_handle *devHandle, libusb_context *ctx) {
	if(devHandle != NULL) {
		libusb_close(devHandle);
	}

	if(ctx != NULL) {
		libusb_exit(ctx);
	}
}
