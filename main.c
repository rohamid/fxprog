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


int ihex_read(FILE *pFile, unsigned char *buf, unsigned int start, unsigned int end) {
	static char line[523];
	fseek(pFile, 0, SEEK_SET);
	
	unsigned int i, byte, line_no = 0, greatest_addr = 0, offset = 0, chunk_len, chunk_addr, chunk_type;
	while(fgets(line, sizeof(line), pFile)) {
		line_no++;

		// Strip off carriage return at the end of line if it exists.
		if(line[strlen(line)] == '\r') {
			line[strlen(line)] = 0;
		}

		// Reading chunk header
		if(sscanf(line, ":%02x%04x%02x", &chunk_len, &chunk_addr, &chunk_type) != 3) {
			free(buf);
			fprintf(stderr, "Error while parsing IHEX at line %d\n", line_no);
			return(-1);
		}
		// Reading chunk data
		if(chunk_type == 2) { // Extended segment address
			unsigned int esa;
			if(sscanf(&line[9], "%04x", &esa) != 1) {
				free(buf);
				fprintf(stderr, "Error while parsing IHEX at line %d\n", line_no);
				return(-1);
			}
			offset = esa * 16;
		}
		if(chunk_type == 4) { // Extended linear address
			unsigned int ela;
			if(sscanf(&line[9], "%04x", &ela) != 1) {
				free(buf);
				fprintf(stderr, "Error while parsing IHEX at line %d\n", line_no);
				return(-1);
			}
			offset = ela * 65536;
		}

		for(i = 9; i < strlen(line)- 1; i+=2) {
			if(sscanf(&line[i], "%02x", &byte) != 1) {
				free(buf);
				fprintf(stderr, "Error while parsing IHEX at line %d\n", line_no);
				return(-1);
			}

			// The only data record have to be processed
			if(chunk_type != 0x00) {
				break;
			}

			if((i - 9) / 2 >= chunk_len) {
				// Respect chunk_len and do not capture checksum as data
				break;
			}

			if((chunk_addr + offset) < start) {
				free(buf);
				fprintf(stderr, "Address %04x is out of range at line %d\n", chunk_addr, line_no);
				return(-1);
			}

			if((chunk_addr + offset + chunk_len) > end) {
				free(buf);
				fprintf(stderr, "Addres %04x + %d is out of range at line %d\n", chunk_addr, chunk_len, line_no);
				return(-1);
			}

			if(chunk_addr + offset + chunk_len > greatest_addr) {
				greatest_addr = chunk_addr + offset + chunk_len;
			}
			buf[chunk_addr + offset - start + (i -9) / 2] = byte;
		}
	}
	return(greatest_addr - start);
}

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

int usb_write(libusb_device_handle *devHandle, const char *label,
			 uint8_t opcode, uint32_t address, const uint8_t *data, uint8_t len) {

	return 0;
}

int main(int argc, char *argv[]) {
	/*
	uint32_t fileSize = 0;
	if(ihex_get_data_size(argv[1], &fileSize) != 0) {
		fprintf(stderr, "Error, cannot get file size\n");
	} else {
		printf("File size: %d bytes\n", fileSize);
	}
	*/

	//ihex_dump_file(argv[1]);

	
	int status;
	libusb_device_handle *devHandle = NULL;
	libusb_device *dev = NULL;

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

	int rv = fx2_write_ihex(devHandle, argv[1]);
	if(rv != 0) {
		printf("Failed to write file %s!\n",argv[1]);
		libusb_close(devHandle);
		libusb_exit(ctx);
		return 1;
	}
	/*
	dev = libusb_get_device(devHandle);

	status = libusb_get_device_speed(dev);
	printf("Device speed before reset: %d\n", status);

	printf("Send FX2LP reset command...\n");
	status = fx2_send_reset(devHandle, true);
	if(status) {
		printf("FX2LP resetted status:%d\n", status);
	} else {
		printf("Unable to reset FX2LP...\n");
	}

	sleep(2);

	printf("Send FX2LP alive command...\n");
	status = fx2_send_reset(devHandle, false);
	if(status) {
		printf("FX2LP alive status:%d\n", status);
	} else {
		printf("Unable to send FX2LP command...\n");
	}

	status = libusb_get_device_speed(dev);
	printf("Device speed after reset: %d\n", status);

	printf("Closing USB device...\n");
	libusb_close(devHandle);
	libusb_exit(ctx);
	*/

	return 0;
}
