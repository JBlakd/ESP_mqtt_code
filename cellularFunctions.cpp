// Non blocking state machine for processing AT commands

#include "cellularFunctions.h"
#include "sdCardFunctions.h"

cellularObj_t cellularObj;

typedef enum {
  cellular_init,
  cellular_wait_RDY,  
  cellular_do_RDY,
  cellular_send_CSQ, 
  cellular_wait_CSQ, 
  cellular_do_CSQ,  
  cellular_send_QMTOPEN,
  cellular_wait_QMTOPEN,
  cellular_do_QMTOPEN,
  cellular_send_QMTCONN,
  cellular_wait_QMTCONN,
  cellular_send_QMTCONNquery,
  cellular_wait_QMTCONNquery,
  cellular_do_QMTCONNquery,
  cellular_send_QMTPUBCmd,
  cellular_wait_QMTPUBCmd,
  cellular_do_QMTPUBCmd,  
  cellular_send_QMTPUBhelloworldMsg,
  cellular_wait_QMTPUBhelloworldMsg,
  cellular_do_QMTPUBhelloworldMsg,
  cellular_send_QMTPUBfileCmd,
  cellular_wait_QMTPUBfileCmd,  
  cellular_send_QMTPUBfile,
  cellular_wait_QMTPUBfile,
  cellular_do_QMTPUBfile,  
  cellular_done
} cellular_handler_state;

// VERY IMPORTANT TO KNOW LENGTH
// Write the contents of the txBuffer to the serial only up to the specified length 
// And then set the rest of the buffer to zero
void txBufferSend(uint8_t bytes_to_send) {
  if (bytes_to_send > TX_BUFFER_LEN) {
    Serial.write("ERROR: bytes_to_send exceeded TX_BUFFER_LEN");
    return;
  }
  
  for (uint8_t i = 0; i <= bytes_to_send - 1; i++) {
    Serial.write(cellularObj.tx_buffer[i]);
    cellularObj.tx_buffer[i] = 0;
  }

  for (uint8_t i = bytes_to_send; i <= TX_BUFFER_LEN-1; i++) {
    cellularObj.tx_buffer[i] = 0;
  }
}

void rxBufferZero(void) {
  for (uint8_t i = 0; i < RX_BUFFER_LEN; i++) {
    cellularObj.rx_buffer[i] = '\0';
  }
}

// We ASSUME that there's content in the Serial buffer, read the first RX_BUFFER_LEN bytes into the rxBuffer
// Zero the rest of the buffer if the message is not long enough to fill the buffer
void rxBufferProcess(void) {
  // null terminate 
  cellularObj.rx_buffer[RX_BUFFER_LEN - 1] = '\0';
  
  // Loop up to and including the penultimate byte (ignore the null terminator)
  for (uint8_t i = 0; i < RX_BUFFER_LEN - 1; i++) {
    if (Serial.available()) {
      cellularObj.rx_buffer[i] = Serial.read();
    } else {
      cellularObj.rx_buffer[i] = '\0';
    }
  }
}

uint8_t isRxBufferContains(char* substr) {
  if (strstr(cellularObj.rx_buffer, substr) == NULL) {
    //Serial.write("does not contain");
    return 0;
  }
  //Serial.write("does contain");
  return 1;
}

// this function is useful for determining whether the buffer is all zero
// and therefore, received no response
uint8_t isRxBufferAllZero(void) {
  for (uint8_t i = 0; i < RX_BUFFER_LEN; i++) {
    if (cellularObj.rx_buffer[i] != '\0') {
      return 0; // not all Zeroes
    }
  }
  return 1; // All zeroes
}

void printRxBuffer(void) {
  for (uint8_t i = 0; i < RX_BUFFER_LEN; i++) {
    Serial.write(cellularObj.rx_buffer[i]);
  }  
}

void cellularHandler() {
  static cellular_handler_state mode;

  switch (mode) {
    case cellular_init:
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 3000;  // Specify how long we will wait in the next state
      mode = cellular_wait_RDY;
      break;
          
    case cellular_wait_RDY:
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_do_RDY; // proceed
      }
      break;
          
    case cellular_do_RDY:
      rxBufferZero();
      rxBufferProcess();
      if ( isRxBufferContains("RDY") ) {
        mode = cellular_send_CSQ; // proceed 
      } else {
        mode = cellular_init; // try again
      }
      break;      
            
    case cellular_send_CSQ:
      Serial.write("AT+CSQ\r\n");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 300;  // Specify how long we will wait in the next state
      mode = cellular_wait_CSQ;
      break;
      
    case cellular_wait_CSQ:
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_do_CSQ; // proceed
      }
      break;
    
    case cellular_do_CSQ:
      rxBufferZero();
      rxBufferProcess();
      //printRxBuffer();
      // If contains +CSQ: 99,99 or NO_RESPONSE, try again
      if (isRxBufferContains("CSQ: 99,99") || isRxBufferAllZero()) {
        mode = cellular_send_CSQ; // try again
      } else {       
        mode = cellular_send_QMTOPEN; // proceed
      }
      break;
    
    case cellular_send_QMTOPEN:
      Serial.write("AT+QMTOPEN=5,\"broker.hivemq.com\",1883\r\n");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 5000;  // Specify how long we will wait in the next state
      mode = cellular_wait_QMTOPEN;
      break;
      
    case cellular_wait_QMTOPEN:
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_do_QMTOPEN; // proceed
      }
      break;
      
    case cellular_do_QMTOPEN:
      rxBufferZero();
      rxBufferProcess();
      if ( isRxBufferContains("QMTOPEN: 5,3") || isRxBufferAllZero() ) {
        mode = cellular_send_QMTOPEN; // try again
      } else {
        mode = cellular_send_QMTCONN; // proceed
      }
      break;

    case cellular_send_QMTCONN:
      Serial.write("AT+QMTCONN=5,\"clientExample\"\r\n");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 5000;  // Specify how long we will wait in the next state
      mode = cellular_wait_QMTCONN;  
      break;
      
    case cellular_wait_QMTCONN:
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_send_QMTCONNquery; // proceed
      }
      break;

    case cellular_send_QMTCONNquery:
      Serial.write("AT+QMTCONN?\r\n");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 500;  // Specify how long we will wait in the next state
      mode = cellular_wait_QMTCONNquery;
      break;      
    
    case cellular_wait_QMTCONNquery:
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_do_QMTCONNquery; // proceed
      }
      break;

    case cellular_do_QMTCONNquery:
      rxBufferZero();
      rxBufferProcess();
      if ( isRxBufferContains("QMTCONN: 5,3") ) {
        mode = cellular_send_QMTPUBCmd; // proceed
      } else {
        mode = cellular_send_QMTCONN; // try again
      }
      break;

    case cellular_send_QMTPUBCmd:
      Serial.write("AT+QMTPUB=5,0,0,0,\"IvanHu\"\r\n");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 500;  // Specify how long we will wait in the next state
      mode = cellular_wait_QMTPUBCmd;
      break;      

    case cellular_wait_QMTPUBCmd:      
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_send_QMTPUBhelloworldMsg; // proceed
      }
      break;

    case cellular_send_QMTPUBhelloworldMsg:
      Serial.printf("%u,", sdFunctionObj.numSegments);
      Serial.printf("%u,", SEGMENT_SIZE);
      Serial.printf("%u", sdFunctionObj.lastSegmentSize);
      Serial.write(char(26));
      Serial.write("\r");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 500;  // Specify how long we will wait in the next state
      mode = cellular_wait_QMTPUBhelloworldMsg;     
      break;
      
    case cellular_wait_QMTPUBhelloworldMsg:
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_do_QMTPUBhelloworldMsg; // proceed
      }
      break;    

    case cellular_do_QMTPUBhelloworldMsg:
      rxBufferZero();
      rxBufferProcess();
      //printRxBuffer();
      if ( isRxBufferContains("QMTPUB: 5,0,0") ) {
        sdFunctionObj.file = SD_MMC.open("/inputjpg_encrypted_encoded"); // Open file
        mode = cellular_send_QMTPUBfileCmd; // proceed
      } else {
        cellularObj.tx_start_time_ms = millis(); // Save the current time
        cellularObj.tx_wait_duration_ms = 500;  // Specify how long we will wait in the next state        
        mode = cellular_wait_QMTPUBhelloworldMsg; // wait a little longer
      }
      break;

    case cellular_send_QMTPUBfileCmd:
      Serial.write("AT+QMTPUB=5,0,0,0,\"IvanHu\"\r\n");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 500;  // Specify how long we will wait in the next state
      mode = cellular_wait_QMTPUBfileCmd;
      break;      

    case cellular_wait_QMTPUBfileCmd:      
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_send_QMTPUBfile; // proceed        
      }
      break;
      
    case cellular_send_QMTPUBfile:
      for (int i = 0; i < SEGMENT_SIZE; i++) {
        if (sdFunctionObj.fileCounter < sdFunctionObj.fileSize) {  
          Serial.write(sdFunctionObj.file.read());
          sdFunctionObj.fileCounter++;
        }
      }
      Serial.write(char(26));
      Serial.write("\r");
      cellularObj.tx_start_time_ms = millis(); // Save the current time
      cellularObj.tx_wait_duration_ms = 500;  // Specify how long we will wait in the next state
      mode = cellular_wait_QMTPUBfile;     
      break;
      
    case cellular_wait_QMTPUBfile:
      if (millis() - cellularObj.tx_start_time_ms > cellularObj.tx_wait_duration_ms) {
        // wait duration has elapsed
        cellularObj.tx_start_time_ms = 0;
        cellularObj.tx_wait_duration_ms = 0;
        mode = cellular_do_QMTPUBfile; // proceed
      }
      break;    

    case cellular_do_QMTPUBfile:
        sdFunctionObj.numSegments--; // decrement numSegments because we will be using this to decide whether to send another pub
        if (sdFunctionObj.numSegments) {
          // if there's still segments to send
          mode = cellular_send_QMTPUBfileCmd;
        } else {
          mode = cellular_done; // done
        }
      break;
          
    case cellular_done:
      if (! (cellularObj.flags_reg & CELLULAR_DONE_FLAG)) {
        sdFunctionObj.file.close();  
        Serial.write("AT+QLTS=2\r\n");
        cellularObj.flags_reg |= CELLULAR_DONE_FLAG;
      }       
      break;
  }
}
