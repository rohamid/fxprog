#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "ihex.h"

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

/* @brief check is fx2 is connected
 *
 */
static inline bool fx2_is_open(libusb_device_handle *usbHandle) {
	return usbHandle;
}


int fx2_write_ram(libusb_device_handle *usbHandle, size_t address, const unsigned char *data, size_t nBytes) {
	// Check if fx2 device is connected
	if(!fx2_is_open(usbHandle)) {
		fprintf(stderr, "fx2_write_ram: Not connected!\n");
		return 1;
	}

	int nErrors = 0;

	const size_t chunkSize = 16;
	const unsigned char *d=data;
	const unsigned char *dEnd=data+nBytes;
	while(d < dEnd) {
		size_t bs = dEnd - d;
		if(bs > chunkSize) bs = chunkSize;
		size_t dlAddr = address + (d - data);
		int rv = libusb_control_transfer(usbHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
										 CMD_FX2LP_REQUEST, dlAddr/*wValue*/, 0/*wIndex*/, (unsigned char*)d/*data*/, bs/*wLength*/, 1000/*timeout*/);
		if(rv < 0) {
			fprintf(stderr, "Writing %zu bytes at 0x%zx: %s\n", bs, dlAddr, libusb_strerror(rv));
			++nErrors;
		}
		d += bs;
	}
	return nErrors;
}

/* @fn : fx2_reset
 * @param devHandle : a pointer to libusb_device_handle struct
 * @param reset : true = put FX2LP in reset state
 * 				 false = put FX2LP to run mode
 * @return true : reset success
 *		  false : reset failed
 */
bool fx2_reset(libusb_device_handle *devHandle, bool reset) {
	int rv = 0;
	uint8_t cpucs = reset;

	// Return number of byte sent upon success
	//rv = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
	//							 CMD_FX2LP_REQUEST, CPUCS_ADDR, 0/*wIndex*/, &cpucs, sizeof(cpucs), TIMEOUT);

	//return (rv == 1 ? true : false);

	rv = fx2_write_ram(devHandle, CPUCS_ADDR, &cpucs, 1);

	return (rv == 0 ? true : false);
}

int ihex_get_size(const char *filePath, uint32_t *dataSize) {
	static char lineBuff[523];
	unsigned int chunkSize, line=0;

	FILE *ihex;

	ihex = fopen(filePath, "r");

	if(ihex == NULL) {
		fprintf(stderr, "Error opening IHEX file\n");
		return 1;
	}

	while(fgets(lineBuff, sizeof(lineBuff), ihex)) {
		// Abandone char but ':', as ':' is the spe
		if(lineBuff[0] != ':')
			continue;

		if(sscanf(lineBuff, ":%02x", &chunkSize) != 1) {
			fprintf(stderr, "Error while parsing IHEX at line %d\n", line);
			rewind(ihex);
			fclose(ihex);
			return 1;
		}

		*dataSize += chunkSize;

		line++;
	}
	rewind(ihex);
	fclose(ihex);
	return 0;
}



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
	if(ihex_get_size(argv[1], &fileSize) != 0) {
		fprintf(stderr, "Error, cannot get file size\n");
	} else {
		printf("File size: %d bytes\n", fileSize);
	}
	*/
	
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
		printf("Error, could not find USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
		return -1;
	}
	printf("Found USB device with 0x%04x:0x%04x\n", FX2LP_VID, FX2LP_PID);
	dev = libusb_get_device(devHandle);

	status = libusb_get_device_speed(dev);
	printf("Device speed before reset: %d\n", status);

	printf("Send FX2LP reset command...\n");
	status = fx2_reset(devHandle, true);
	if(status) {
		printf("FX2LP resetted status:%d\n", status);
	} else {
		printf("Unable to reset FX2LP...\n");
	}

	sleep(2);

	printf("Send FX2LP alive command...\n");
	status = fx2_reset(devHandle, false);
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
	

	return 0;
}
