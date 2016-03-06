#if 0
#include <mbed.h>

#include "hal_types.h"
#include "cc112x_spi.h"

// pins used for CC1120 radio communication
#define RADIO_SPI_MOSI  PB_15
#define RADIO_SPI_MISO  PB_14
#define RADIO_SPI_SCLK  PB_13
#define RADIO_SPI_CS    PB_1

// define radio SPI interface
SPI radioSpi(RADIO_SPI_MOSI, RADIO_SPI_MISO, RADIO_SPI_SCLK);
DigitalOut radioSpiCs(RADIO_SPI_CS);

#define RADIO_RESET PB_10
#define RADIO_GPIO_0 PB_5
#define RADIO_GPIO_2 PB_4
#define RADIO_GPIO_3 PA_8

DigitalOut radioReset(RADIO_RESET);
DigitalOut radioGPIO_0(RADIO_GPIO_0);
DigitalOut radioGPIO_2(RADIO_GPIO_2);
DigitalOut radioGPIO_3(RADIO_GPIO_3);

void trxSpiInit(void) {
  // set reset and cs HI
  radioReset = 1;
  radioSpiCs = 1;

  // configure SPI for radio communication
  radioSpi.format(8, 0);
  radioSpi.frequency(4000000);
}

rfStatus_t trx8BitRegAccess(uint8_t accessType, uint8_t addrByte, uint8_t* pData, uint16_t len) {
  rfStatus_t out;

  radioSpiCs = 0;
  out = radioSpi.write(accessType | addrByte);

  trxReadWriteBurstSingle(accessType|addrByte, pData, len);
  radioSpiCs = 1;

  return out;
}

rfStatus_t trx16BitRegAccess(uint8_t accessType, uint8_t extAddr, uint8_t regAddr, uint8_t* pData, uint8_t len) {
  rfStatus_t out;

  radioSpiCs = 0;
  out = radioSpi.write(accessType | extAddr);
  radioSpi.write(regAddr);

  trxReadWriteBurstSingle(accessType|extAddr, pData, len);
  radioSpiCs = 1;

  return out;
}

rfStatus_t trxSpiCmdStrobe(uint8_t cmd) {
  rfStatus_t out;

  radioSpiCs = 0;
  out = radioSpi.write(cmd);
  radioSpiCs = 1;

  return out;
}

void trxReadWriteBurstSingle(uint8_t addr, uint8_t* pData, uint16_t len) {
  if (addr & RADIO_READ_ACCESS) {
    if (addr & RADIO_BURST_ACCESS) {
      for (int i=0; i < len; i++) {
        *pData = radioSpi.write(0x00);
        pData++;
      }
    } else {
      *pData = radioSpi.write(0x00);
    }
  } else {
    if (addr & RADIO_BURST_ACCESS) {
      for (int i=0; i < len; i++) {
        radioSpi.write(*pData);
        pData++;
      }
    } else {
      radioSpi.write(*pData);
    }
  }
}

void halRfWriteReg(uint16_t addr, uint8_t data) {
  cc112xSpiWriteReg(addr, &data, 1);
}

#endif
