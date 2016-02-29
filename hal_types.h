#ifndef __HAL_TYPES_H
#define __HAL_TYPES_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

union cc1120_status {
  struct {
    uint8_t reserved : 4;
    uint8_t STATE : 3;
    uint8_t CHIP_RDY : 1;
  } fields;
  uint8_t value;
};

#define RADIO_BURST_ACCESS   0x40
#define RADIO_SINGLE_ACCESS  0x00
#define RADIO_READ_ACCESS    0x80
#define RADIO_WRITE_ACCESS   0x00

/* Bit fields in the chip status byte */
#define STATUS_CHIP_RDYn_BM             0x80
#define STATUS_STATE_BM                 0x70
#define STATUS_FIFO_BYTES_AVAILABLE_BM  0x0F

typedef uint8_t rfStatus_t;

void trxSpiInit(void);
rfStatus_t trx8BitRegAccess(uint8_t accessType, uint8_t addrByte, uint8_t* pData, uint16_t len);
rfStatus_t trx16BitRegAccess(uint8_t accessType, uint8_t extAddr, uint8_t regAddr, uint8_t* pData, uint8_t len);
rfStatus_t trxSpiCmdStrobe(uint8_t cmd);

void trxReadWriteBurstSingle(uint8_t addr, uint8_t* pData, uint16_t len);

void halRfWriteReg(uint16_t addr, uint8_t data);


#endif
