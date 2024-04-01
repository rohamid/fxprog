#include "ihex.h"

#define HI(x)	(((x) & 0xff00) >> 8)
#define LO(x)	((x) & 0xff)


/*
 * @brief calculate record's checksum
 */
static unsigned char ihex_get_checksum(hex_record_t *record) {
	unsigned char checksum = record->length + record->address + (record->address >> 8) + record->type;

	for(int i = 0; i < record->length; i++) {
		checksum += record->data[i];
	}

	return (~checksum) + 1;
}

/*
 * @brief calculate data size of given ihex file
 */
int ihex_get_data_size(const char *filePath, unsigned int *dataSize) {
	unsigned int chunkSize, line=0;
	static char lineBuff[523];

	// Open ihex file
	FILE *pFile = fopen(filePath, "r");
	if(pFile == NULL) {
		fprintf(stderr, "Error opening IHEX file\n");
		return(1);
	}

	// Read ihex file line by line
	while(fgets(lineBuff, sizeof(lineBuff), pFile)) {
		// Ignore char but ':', as ':' is the first character 
		// ihex specifier
		if(sscanf(lineBuff, ":%02x", &chunkSize) != 1) {
			fprintf(stderr, "Error while parsing IHEX at line %d\n", line);
			if(pFile) {
				rewind(pFile);
				fclose(pFile);
				return(1);
			}
		}
		// Accumulate data size
		*dataSize += chunkSize;
		line++;
	}
	// Close and put the file pointer to the 1st location
	if(pFile) {
		rewind(pFile);
		fclose(pFile);
	}
	return(0);
}

/*
 * @brief parsing single line of ihex record, save the result to hex_record_t 
 */
int ihex_parse_record(const char *line, hex_record_t *record) {
	int byteCount, recordType;
	unsigned int address;
	int i;

	// Check if the line starts with ':'
	if(line[0] != ':') {
		printf("ihex_parse_record: Not a valid intel hex record!\n");
		return(-1);
	}

	// Parse record fields
	if(sscanf(line, ":%2x%4x%2x", &byteCount, &address, &recordType) != 3) {
		printf("ihex_parse_record: Error parsing hex file!\n");
		return(-1);
	}

	// Allocate memory for data
	record->data = (unsigned char*)malloc(byteCount * sizeof(unsigned char));
	if(record->data == NULL) {
		return(-2);	// Memory allocation failed
	}

	// Populate record fields
	record->address = address;
	record->length = byteCount;
	record->type = recordType;

	// Parse data bytes
	for(i = 0; i < byteCount; i++) {
		if(sscanf(line + 9 + i * 2, "%2x", (unsigned int*)&record->data[i]) != 1) {
			printf("Failed to get data!\n");
			free(record->data);
			return(-1);
		}
	}
	// Parse checksum
	if(sscanf(line+9+byteCount*2, "%2x", &record->checksum) != 1) {
		printf("Failed to get line checksum!\n");
		free(record->data);
		return(-1);
	}
	// Compare checksum
	if(record->checksum != ihex_get_checksum(record)) {
		printf("Failed: checksum missmatch!\n");
		free(record->data);
		return(-1);
	}

	return(0); 	// Parsing succesfull
}

/*
 * @brief dump ihex file
 */
void ihex_dump_file(const char *fileName) {
	FILE *pFile;
	char line[MAX_RECORD_SIZE];
	hex_record_t record;
	int lineNumber = 0;

	// Open file for reading
	pFile = fopen(fileName, "r");
	if(pFile == NULL) {
		printf("ihex_dump_file: Error opening file!\n");
		return;
	}

	// Read and parse each line of the file
	while(fgets(line, MAX_RECORD_SIZE, pFile) != NULL) {
		lineNumber++;
		if(ihex_parse_record(line, &record) == 0) {
			printf("[0x%04x] ", record.address);

			for(int i=0; i<record.length; i++) {
				printf("0x%02x ", record.data[i]);

				if((i + 1) % 16 == 0 || (i + 1) == record.length) {
					printf("\n");
					if((i + 1) != record.length)
						printf("[0x%04x] ", record.address + (i + 1));
				}
			}

			free(record.data);
		} else {
			printf("ihex_dump_file: Error parsing line %d: Not a valid Intel HEX record\n", lineNumber);
		}
	}
	fclose(pFile);
}
