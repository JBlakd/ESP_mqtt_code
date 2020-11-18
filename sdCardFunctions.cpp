#include "sdCardFunctions.h"

sdFunctionObj_t sdFunctionObj;

uint8_t sdInit(void) {
  sdFunctionObj.fileCounter = 0; 
  
  if(!SD_MMC.begin()){
    return 1;
  }
}

void getFileSizeSegment(fs::FS &fs, char* dir) {
  sdFunctionObj.file = fs.open(dir);
  uint32_t counter = 0;
  while (sdFunctionObj.file.available()) {
    sdFunctionObj.file.read();
    counter++;
  }
  sdFunctionObj.fileSize = counter;
  sdFunctionObj.numSegments = (sdFunctionObj.fileSize / SEGMENT_SIZE) + 1;
  sdFunctionObj.lastSegmentSize = sdFunctionObj.fileSize % SEGMENT_SIZE;
  sdFunctionObj.file.close();    
}

void sdTest(fs::FS &fs) {
  sdFunctionObj.file = fs.open("/inputjpg_encrypted_encoded");
  uint32_t fileSize = 0;
  while (sdFunctionObj.file.available()) {
    sdFunctionObj.file.read();
    fileSize++;
  }
  Serial.printf("\r\nfileSize = %u\r\n", fileSize);
  sdFunctionObj.file.close();  
  
  sdFunctionObj.file = fs.open("/inputjpg_encrypted_encoded");
  uint8_t numSegments = (fileSize / SEGMENT_SIZE) + 1;
  Serial.printf("\r\nSend %u segments\r\n", numSegments);

  int fileCounter = 0;
  for (int j = 0; j < numSegments; j++) {
    Serial.printf("\r\nSegment %u:\r\n", j);
    for (int i = 0; i < SEGMENT_SIZE; i++) {
      if (fileCounter < fileSize) {  
        Serial.write(sdFunctionObj.file.read());
        fileCounter++;
      }
    }
  }
  sdFunctionObj.file.close();
}

void aesEncode(fs::FS &fs, char* srcDir, char* destDir) {
  //Serial.print("Commencing aesEncode\r\n");
  sdFunctionObj.file = fs.open(srcDir, "rb");
  sdFunctionObj.fileEncoded = fs.open(destDir, FILE_WRITE);
  
  if(!sdFunctionObj.fileEncoded){
      //Serial.println("Failed to open file for writing");
      return;
  }
  
  char * key = AES_KEY;
  uint8_t paddingCount = 0;
  
  while (sdFunctionObj.file.available()) {
    uint8_t buf[AES_CHUNK]; 
    unsigned char encryptedBuf[AES_CHUNK];
    mbedtls_aes_context aes; 
    
    // Write AES_CHUNK bytes into buf
    for (uint8_t i = 0; i < AES_CHUNK; i++) {
      if (sdFunctionObj.file.available()) {
        buf[i] = sdFunctionObj.file.read();
      } else {
        // Padding needed
        buf[i] = '0xFF';
        paddingCount++;
      }
    }  
    
    // AES128 encrypt buf into encryptedBuf
    mbedtls_aes_init( &aes );
    mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)buf, encryptedBuf);
    mbedtls_aes_free( &aes );
    
    for (uint8_t i = 0; i < AES_CHUNK; i++) {
      sdFunctionObj.fileEncoded.write(encryptedBuf[i]);
      //Serial.print(encodedBuf.charAt(i));
    }    
  }
  
  //Serial.print("File encrypted\r\n");
  sdFunctionObj.fileEncoded.close();
  sdFunctionObj.file.close();
}

void base64Encode(fs::FS &fs, char* srcDir, char* destDir) {
  //Serial.print("Commencing base64Encode\r\n");
  sdFunctionObj.file = fs.open(srcDir, "rb");
  sdFunctionObj.fileEncoded = fs.open(destDir, FILE_WRITE);
    
  if(!sdFunctionObj.fileEncoded){
      //Serial.println("Failed to open file for writing");
      return;
  }

  uint8_t buf[BASE_64_CHUNK];  
  size_t outputLength;
  unsigned char * encoded;

  while (sdFunctionObj.file.available()) {
    // Write BASE_64_CHUNK bytes into buf
    for (uint8_t i = 0; i < BASE_64_CHUNK; i++) {
      if (sdFunctionObj.file.available()) {
        buf[i] = sdFunctionObj.file.read();
      }
    }

    // Encode buf into encoded
    encoded = base64_encode((const unsigned char *)buf, BASE_64_CHUNK, &outputLength);
           
    for (uint8_t i = 0; i < outputLength; i++) {
      sdFunctionObj.fileEncoded.write(encoded[i]);
      //Serial.print(encodedBuf.charAt(i));
    }
    
    free(encoded);
  }

  sdFunctionObj.fileEncoded.close();
  sdFunctionObj.file.close();
}
