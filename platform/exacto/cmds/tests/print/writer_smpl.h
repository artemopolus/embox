#ifndef EXACTO_CMDS_TEST_PRINT_WRITER_SMPL_H
#define EXACTO_CMDS_TEST_PRINT_WRITER_SMPL_H 
#include <stdint.h>

extern void addDataToWrite( uint8_t * data, uint16_t datalen);
extern void addDataToFile( uint8_t file_id, uint8_t * data, uint16_t datalen);
extern void printReaderData();
extern void openFileSD();
extern uint8_t isReadyToWrite();
extern void blockWrite();
extern void unBlockWrite();
extern void readTestFile();

#endif