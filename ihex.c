#include "ihex.h"

/*
 * @brief calculate data size of given ihex file
 */
int ihex_get_data_size(const char *filePath, unsigned int *dataSize) {
	unsigned int chunkSize, line=0;
	static char lineBuff[523];

	// Open ihex file
	FILE *ihex = fopen(filePath, "r");
	if(ihex == NULL) {
		fprintf(stderr, "Error opening IHEX file\n");
		return(1);
	}

	// Read ihex file line by line
	while(fgets(lineBuff, sizeof(lineBuff), ihex)) {
		// Ignore char but ':', as ':' is the first character 
		// ihex specifier
		if(sscanf(lineBuff, ":%02x", &chunkSize) != 1) {
			fprintf(stderr, "Error while parsing IHEX at line %d\n", line);
			if(ihex) {
				rewind(ihex);
				fclose(ihex);
				return(1);
			}
		}
		// Accumulate data size
		*dataSize += chunkSize;
		line++;
	}
	// Close and put the file pointer to the 1st location
	if(ihex) {
		rewind(ihex);
		fclose(ihex);
	}
	return(0);
}

int ihex_parse_record(char *line, hex_record_t *record) {
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

	line++;
	
	// Parse data bytes
	for(i = 0; i < byteCount; i++) {
		sscanf(line + 8 + i * 2, "%2x", &record->data[i]);
	}

	return(0); 	// Parsing succesfull
}

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
			// Dump record contents
			printf("ihex_dump_file: Line %d - Address: 0x%04x, Length: %d bytes, Data: ", lineNumber, record.address, record.length);
			for(int i = 0; i < record.length; i++) {
				printf("%02x ", record.data[i]);
			}
			printf("\n");

			// Free memory allocated for data
			free(record.data);
		} else {
			printf("ihex_dump_file: Error parsing line %d: Not a valid Intel HEX record\n", lineNumber);
		}
	}
	fclose(pFile);
}
