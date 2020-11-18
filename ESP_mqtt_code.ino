#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include "cellularFunctions.h"
#include "sdCardFunctions.h"
#include "cameraFunctions.h"

void setup() {
  Serial.begin(FAST_CATM1_BAUDRATE); 
  sdInit();
  configInitCamera();
  takeSavePhoto("/input.jpg", SD_MMC);
  aesEncode(SD_MMC, "/input.jpg", "/inputjpg_encrypted");
  base64Encode(SD_MMC, "/inputjpg_encrypted", "/inputjpg_encrypted_encoded");
  getFileSizeSegment(SD_MMC, "/inputjpg_encrypted_encoded");  
}

void loop() {
  cellularHandler();  
}
