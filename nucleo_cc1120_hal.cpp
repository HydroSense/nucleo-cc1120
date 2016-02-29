#include <mbed.h>

#include "hal_types.h"
#include "cc112x_spi.h"

rfStatus_t trx8BitRegAccess(uint8_t accessType, uint8_t addrByte, uint8_t* pData, uint16_t len) {
  uint8_t out;

  radioSpiCs = 0;
  status.value = radioSpi.write(accessType | addrByte);

  trxReadWriteBurstSingle(accessType|addrByte, pData, len);
  radioSpiCs = 1;

  return out;
}

rfStatus_t trx16BitRegAccess(uint8_t accessType, uint8_t extAddr, uint8_t regAddr, uint8_t* pData, uint8_t len) {
  uint8_t out;

  radioSpiCs = 0;
  status.value = radioSpi.write(RADIO_READ_ACCESS | EXTENDED_ADDRESS);
  radioSpi.write(extended_address);
  out = radioSpi.write(0xff);

  radioSpiCs = 1;

  return out;
}

rfStatus_t trxSpiCmdStrobe(uint8_t cmd) {
  radioSpiCs = 0;
  status.value = radioSpi.write(RADIO_READ_ACCESS | cmd);
  radioSpiCs = 1;
}

void trxReadWriteBurstSingle(uint8_t addr, uint8_t* pData, uint16_t len) {
  uint16_t i;
  if (addr & RADIO_READ_ACCESS) {
    if (addr & RADIO_BURST_ACCESS) {
      for (int i=0; i < len; i++) {
        *pData = radioSpi.write(0x00);
        pData++
      }
    } else {
      *pData = radioSpi.write(0x00);
    }
  } else {
    if (addr & RADIO_BURST_ACCESS) {
      for (int i=0; i < len; i++) {
        *pData = radioSpi.write(0x00);
        pData++;
      }
    } else {
      *pData = radioSpi.write(0x00);
    }
  }
}
