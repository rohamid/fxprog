#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "ihex.h"
#include "usbdev.h"
#include "usb_fx2.h"

#include <libusb-1.0/libusb.h>

#define FX2LP_VID	0x04b4
#define FX2LP_PID	0x8613

#define CMD_FX2LP_REQUEST	0xA0
#define CPUCS_ADDR			0xE600
#define TIMEOUT				250

// Libusb context
libusb_context *ctx = NULL;

// Global usb device handle
libusb_device_handle *g_usbDevHandle = NULL;

bool fx2_open(int vid, int pid) {
	libusb_device **usb_devices;
	int rv, count;

	count = libusb_get_device_list(ctx, &usb_devices);
	printf("Devices count:%d\n", count);

	for(int i=0; i<count; i++) {
		libusb_device *dev = usb_devices[i];
		struct libusb_device_descriptor devDesc;

		rv = libusb_get_device_descriptor(dev, &devDesc);
		if(rv != 0) printf("Failed to get device descriptor\n");
	}

	libusb_free_device_list(usb_devices, 1);
	return true;
}

int main(int argc, char *argv[]) {
	int status;
	libusb_device_handle *devHandle = NULL;

	status = libusb_init(&ctx);
	if(status != 0) {
		printf("Error : %s\n", libusb_error_name(status));
		return -status;
	}
	//fx2_open(FX2LP_VID, FX2LP_PID);

	devHandle = libusb_open_device_with_vid_pid(ctx, FX2LP_VID, FX2LP_PID);
	if(devHandle == NULL) {
		printf("Error at main, could not find USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
		return -1;
	}
	printf("Found USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);

	uint32_t fileSize = 0;
	int fileStat = ihex_get_data_size(argv[1], &fileSize);
	if(fileStat != 0) {
		printf("Failed to get file size!\n");
		goto on_failed;
	}
	printf("Writing %d bytes\n", fileSize);
	int rv = fx2_write_ihex(devHandle, argv[1]);
	if(rv != 0) {
		printf("Failed to write file %s!\n",argv[1]);
		goto on_failed;
	}
	printf("Done!\n");
	// Success
	return 0;

on_failed:
	libusb_close(devHandle);
	libusb_exit(ctx);
	return 1;
}
