#ifndef IHEX_H_
#define IHEX_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_RECORD_SIZE		128

typedef struct {
	unsigned int address;
	int length;
	int type;
	unsigned char *data;
	unsigned int checksum;
}hex_record_t;

int ihex_get_data_size(const char *filePath, unsigned int *dataSize);
int ihex_parse_record(const char *line, hex_record_t *record);
void ihex_dump_file(const char *fileName);




#endif /* End of Ihex */
