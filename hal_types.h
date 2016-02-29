#ifndef __HAL_TYPES_H
#define __HAL_TYPES_H

#include <stdint.h>

#define RADIO_BURST_ACCESS   0x40
#define RADIO_SINGLE_ACCESS  0x00
#define RADIO_READ_ACCESS    0x80
#define RADIO_WRITE_ACCESS   0x00

typedef uint8_t rfStatus_t;

rfStatus_t trx8BitRegAccess(uint8_t accessType, uint8_t addrByte, uint8_t* pData, uint16_t len);
rfStatus_t trx16BitRegAccess(uint8_t accessType, uint8_t extAddr, uint8_t regAddr, uint8_t* pData, uint8_t len);
rfStatus_t trxSpiCmdStrobe(uint8_t cmd);

void trxReadWriteBurstSingle(uint8_t addr, uint8_t* pData, uint16_t len);

#endif
