#ifndef __SD_CARD_FUNCTIONS_H__
#define __SD_CARD_FUNCTIONS_H__

#define SEGMENT_SIZE 1500
// Bytes to encode into Base64 at a time
#define BASE_64_CHUNK 120
#define AES_CHUNK 16
#define AES_KEY "ivanhuIVANHUivhu" 

#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "mbedtls/aes.h"
extern "C" {
#include "crypto/base64.h"
}

typedef struct {
  uint8_t flags_reg;
  uint32_t fileSize;
  uint16_t numSegments;
  uint32_t lastSegmentSize;
  uint32_t fileCounter; // variable for keeping track of how many bytes have been read
  File file;
  File fileEncoded;
} sdFunctionObj_t;

extern sdFunctionObj_t sdFunctionObj;

uint8_t sdInit(void);
void getFileSizeSegment(fs::FS &fs, char* dir);
void sdTest(fs::FS &fs);
void aesEncode(fs::FS &fs, char* srcDir, char* destDir);
void base64Encode(fs::FS &fs, char* srcDir, char* destDir);

#endif
