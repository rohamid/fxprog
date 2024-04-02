#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "ihex.h"
#include "usbdev.h"
#include "usb_fx2.h"

#include <libusb-1.0/libusb.h>

#define PROGRAM_RELEASE_DATE	"01042024"
#define PROGRAM_VERSION			"1.0"
#define PROGRAM_NOTE			""

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

void print_usage_and_exit() {
	
}

void print_version_and_exit() {
	fprintf(stderr, "fxprog %s-%s\n%s", PROGRAM_RELEASE_DATE, PROGRAM_VERSION, PROGRAM_NOTE);
	exit(1);
}

int main(int argc, char *argv[]) {
	int opt;
	char *fileName = NULL;

	while((opt = getopt(argc, argv, "f:i")) != -1) {
		switch (opt) {
			case 'f':
				fileName = optarg;
			break;

			case 'i':
				print_version_and_exit();
			break;

			default:
				fprintf(stderr, "Usage: %s -f filename\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if(fileName == NULL) {
		fprintf(stderr, "Usage: %s -f filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}


	// usb device handle
	libusb_device_handle *devHandle = NULL;

	// Init libusb
	usb_init(&ctx);

	// Open fx2lp
	if(!usb_open(&devHandle, &ctx, FX2LP_VID, FX2LP_PID))
		goto on_failed;
	/*
	devHandle = libusb_open_device_with_vid_pid(ctx, FX2LP_VID, FX2LP_PID);
	if(devHandle == NULL) {
		printf("Error at main, could not find USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
		return -1;
	}
	printf("Found USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
	*/
	uint32_t fileSize = 0;
	int fileStat = ihex_get_data_size(fileName, &fileSize);
	if(fileStat != 0) {
		printf("Failed to get file size!\n");
		goto on_failed;
	}
	printf("Writing %d bytes\n", fileSize);
	int rv = fx2_write_ihex(devHandle, fileName);
	if(rv != 0) {
		printf("Failed to write file %s!\n",argv[1]);
		goto on_failed;
	}
	printf("Done!\n");

	// Success
	usb_close(devHandle, ctx);

	return 0;

on_failed:
	usb_close(devHandle, ctx);
	return 1;
}
