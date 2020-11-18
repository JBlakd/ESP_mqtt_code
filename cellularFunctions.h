#ifndef __CELLULAR_FUNCTIONS_H__
#define __CELLULAR_FUNCTIONS_H__

#include "Arduino.h"
#include <stdint.h>
#include <string.h>

#define TX_BUFFER_LEN 129
#define RX_BUFFER_LEN 129
#define FAST_CATM1_BAUDRATE 115200

// flag_reg defines
#define CELLULAR_DONE_FLAG 0x01

typedef struct {
  char tx_buffer[TX_BUFFER_LEN];
  char rx_buffer[RX_BUFFER_LEN];
  unsigned long tx_wait_duration_ms;
  unsigned long tx_start_time_ms;
  uint8_t flags_reg = 0;
} cellularObj_t;  

extern cellularObj_t cellularObj;

void txBufferSend(uint8_t bytes_to_send);
void rxBufferZero(void);
void rxBufferProcess(void);
void cellularHandler(void);
uint8_t isRxBufferContains(char* substr);
uint8_t isRxBufferAllZero(void);
void printRxBuffer(void);

#endif
