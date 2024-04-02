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
	libusb_device **usb_devices;
	int rc, count;

	count = libusb_get_device_list(*ctx, &usb_devices);
	usb_error_check(count, "libusb_get_device_list()");

	for(int i = 0; i < count; i++) {
		libusb_device *dev = usb_devices[i];
		struct libusb_device_descriptor desc;

		rc = libusb_get_device_descriptor(dev, &desc);
		usb_error_check(rc, "libusb_get_device_descriptor()");

		if(desc.idVendor == vid && desc.idProduct == pid) {

			rc = libusb_open(dev, devHandle);
			usb_error_check(rc, "libusb_open()");
			libusb_free_device_list(dev, 1);
			return true;
		}
	}

	libusb_free_device_list(usb_devices, 1);

	if(!devHandle) {
		return false;
	}

	libusb_set_auto_detach_kernel_driver(*devHandle, 1);

	rc = libusb_claim_interface(*devHandle, 0);
	usb_error_check(rc, "libsub_claim_interface()");

	return true;

}

void usb_close(libusb_device_handle *devHandle, libusb_context *ctx) {
	if(devHandle != NULL) {
		libusb_close(devHandle);
	}

	if(ctx != NULL) {
		libusb_exit(ctx);
	}
}
