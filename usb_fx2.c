#include "usb_fx2.h"

/*
 * @brief check if fx2 is open
 */
static inline bool fx2_is_open(libusb_device_handle *devHandle) {
	return(devHandle);
}

/*
 * write a chunk of data to the fx2 ram
 */
int fx2_write_ram(libusb_device_handle *devHandle, size_t address, const unsigned char *data, size_t nBytes) {
	// Check if device is connected
	if(!fx2_is_open(devHandle)) {
		fprintf(stderr, "fx2_write_ram: Not connected!\n");
		return(1);
	}

	// Num of error tries
	int nErrors = 0;

	const size_t chunkSize = 16;	// only 16 bytes are allower per transfer
	const unsigned char *d = data;
	const unsigned char *dEnd = data + nBytes;
	while(d < dEnd) {
		size_t byteSize = dEnd - d;
		if(byteSize > chunkSize) byteSize = chunkSize;
		size_t dlAddr = address + (d - data);
		int rv = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
										 FX2LP_CMD_REQUEST, 
										 dlAddr/*wValue*/, 
										 0/*wIndex*/, 
										 (unsigned char*)d/*data*/, 
										 byteSize/*wLength*/, 
										 FX2LP_CONTROL_TIMEOUT/*timeout*/);
		if(rv < 0) {
			fprintf(stderr, "Writing %zu bytes at 0x%zx: %s\n", byteSize, dlAddr, libusb_strerror(rv));
			++nErrors;
		}
		d += byteSize;
	}
	return(nErrors);
}

/*
 * @brief read a chunk of data from fx2
 */
int fx2_read_ram(libusb_device_handle *devHandle, size_t address, unsigned char *data, size_t nBytes) {
	// Check if fx2 is available and opened
	if(!fx2_is_open(devHandle)) {
		fprintf(stderr, "fx2_read_ram: Not connected!\n");
		return(1);
	}

	int nErrors = 0;

	const size_t chunkSize = 16;	// Only 16 bytes maximum allowed per transfer

	unsigned char *d = data;
	unsigned char *dEnd = data + nBytes;

	while(d < dEnd) {
		size_t byteSize = dEnd - d;
		if(byteSize > chunkSize) byteSize = chunkSize;
		size_t rdAddr = address + (d - data);
		int rv = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
										 FX2LP_CMD_REQUEST/*bRequest*/, 
										 rdAddr/*wValue*/, 
										 0/*wIndex*/, 
										 data/*data*/, 
										 nBytes/*wLength*/, 
										 FX2LP_CONTROL_TIMEOUT/*timeout*/);
		if(rv < 0) {
			fprintf(stderr, "Reading %zu bytes at 0x%zx: %s\n", byteSize, rdAddr, libusb_strerror(rv));
			++nErrors;
		}
		d += byteSize;
	}
	return(nErrors);
}

/*
 * @brief send reset signal to the fx2
 */
bool fx2_send_reset(libusb_device_handle *devHandle, bool reset) {
	int rv = 0;	// libusb return status
	uint8_t cpucs = reset;	// Reset state

	rv = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
								 FX2LP_CMD_REQUEST, 
								 FX2LP_CPUCS_ADDR/*wValue*/, 
								 0/*wIndex*/, 
								 &cpucs/*data*/, 
								 1/*wLength*/, 
								 FX2LP_CONTROL_TIMEOUT/*timeout*/);

	//rv = fx2_write_ram(devHandle, FX2LP_CPUCS_ADDR, &cpucs, 1);
	return(rv == 0 ? true : false);
}

/*
 * @brief write single line of ihex record to the fx2lp ram 
 */
int fx2_write_ihex_line(libusb_device_handle *devHandle, const char *lineRecord) {
	hex_record_t parseRecord = {0};
	int rv = ihex_parse_record(lineRecord, &parseRecord);
	if(rv != 0) {
		printf("Failed to parse record!\n");
		return -1;
	}

	rv = fx2_write_ram(devHandle, parseRecord.address, parseRecord.data, parseRecord.length);
	if(rv != 0) {
		printf("Failed to write line record!\n");
		free(parseRecord.data);
		return -1;
	}
	// free allocated memory
	free(parseRecord.data);
	return 0;
}

/*
 * @brief write ihex file to the fx2lp ram 
 */
int fx2_write_ihex(libusb_device_handle *devHandle, const char *filePath) {
	FILE *pFile = fopen(filePath, "r");
	if(pFile == NULL) {
		printf("Failed to open file %s!\n", filePath);
		return 1;
	}

	char line[523];
	int lineCount = 1;
	// Put fx2lp to the reset state
	fx2_send_reset(devHandle, true);

	while(fgets(line, sizeof(line), pFile)) {
		int rv = fx2_write_ihex_line(devHandle, line);
		if(rv != 0) {
			printf("Failed to write file at line [%d]!\n", lineCount);
			if(pFile) {
				fclose(pFile);
				return -1;
			}
		}

		lineCount++;
	}
	// printf("Done writing\n");

	// Put fx2lp out of reset
	fx2_send_reset(devHandle, false);

	rewind(pFile);
	fclose(pFile);
	return 0;
}



