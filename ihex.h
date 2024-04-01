#ifndef IHEX_H_
#define IHEX_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_RECORD_SIZE		128

// Struct to hold single line of ihex record
typedef struct {
	int length;				/* data length (not entire record length) */
	unsigned int address; 	/* address */
	int type; 				/* record type */
	unsigned char *data; 	/* data buffer */
	unsigned int checksum;	/* record checksum */
}hex_record_t;

int ihex_get_data_size(const char *filePath, unsigned int *dataSize);
int ihex_parse_record(const char *line, hex_record_t *record);
void ihex_dump_file(const char *fileName);

#endif /* End of Ihex */
