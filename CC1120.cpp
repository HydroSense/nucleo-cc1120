#include "cc112x_spi.h"
#include "CC1120.hpp"

CC1120Error CC1120Errno;

/* Private Functions */

void CC1120::flushTxFifo() {
  this->strobe(CC112X_SFTX);
}

void CC1120::flushRxFifo() {
  this->strobe(CC112X_SFRX);
}

rfStatus_t CC1120::strobe(uint8_t cmd) {
  rfStatus_t status;

  mRadioSpiCs.write(0);
  status = mRadioSpi.write(cmd);
  mRadioSpiCs.write(1);

  return status;
}

int CC1120::regAccess(uint8_t accessType, uint16_t addr, uint8_t* pData, uint8_t len) {
  uint8_t extAddr = addr >> 8;
  uint8_t tmpAddr = addr & 0x00FF;

  if (extAddr == 0x2F) {              // extended address case
    mRadioSpiCs.write(0);

    mRadioSpi.write((accessType|RADIO_BURST_ACCESS) | extAddr);
    mRadioSpi.write(tmpAddr);

    this->readWriteBurst(accessType, pData, len);

    mRadioSpiCs.write(1);
  } else if (extAddr == 0x00) {       // normal address case
    mRadioSpiCs.write(0);

    mRadioSpi.write((accessType|RADIO_BURST_ACCESS) | tmpAddr);

    this->readWriteBurst(accessType, pData, len);

    mRadioSpiCs.write(1);
  } else {
    CC1120Errno = BAD_REG_ACCESS;
    return -1;
  }

  CC1120Errno = OK;
  return 0;
}

void CC1120::readWriteBurst(uint8_t type, uint8_t* pData, uint8_t len) {
  if (type & RADIO_READ_ACCESS) {
    for (int i=0; i < len; i++) {
      pData[i] = mRadioSpi.write(0x00);
    }
  } else {
    for (int i=0; i < len; i++) {
      mRadioSpi.write(pData[i]);
    }
  }
}


/* Public Functions */
/*
CC1120::CC1120(PinName spiMosiPin, PinName spiMisoPin, PinName spiSClkPin, PinName spiCSPin, PinName resetPin) :
  mRadioSpi(spiMosiPin, spiMisoPin, spiSClkPin),
  mRadioSpiCs(spiCSPin),
  mRadioReset(resetPin) {

  // hold the radio reset high until we begin
  mRadioReset.write(0);
  mRadioState = CC1120_STATE_UNINITIALIZED;
}
*/

CC1120::CC1120(SPI& radioSpi, DigitalOut& spiCs, DigitalOut& reset):
  mRadioSpi(radioSpi),
  mRadioSpiCs(spiCs),
  mRadioReset(reset) {

  // hold the radio reset high until we begin
  mRadioReset.write(0);
  mRadioSpiCs.write(1);
  mRadioState = CC1120_STATE_UNINITIALIZED;
}

void CC1120::begin(void) {
  // deassert the CS while turning off reset
  mRadioReset.write(1);

  // set the spi format and frequency (4 MHz)
  mRadioSpi.format(8, 0);
  mRadioSpi.frequency(4000000);

  this->getState();

  // flush the FIFOs
  this->flushRxFifo();
  this->flushTxFifo();
}

void CC1120::end(void) {
  // leave the reset pin low, triggering continual reset mode
  mRadioReset.write(0);
}

int CC1120::setRegister(uint16_t reg, uint8_t val) {
  return this->regAccess(RADIO_WRITE_ACCESS, reg, &val, 1);
}

int CC1120::getRegister(uint16_t reg, uint8_t* val) {
  return this->regAccess(RADIO_READ_ACCESS, reg, val, 1);
}

CC1120State CC1120::getState(void) {
  union cc1120_status status;
  status.value = this->strobe(CC112X_SNOP);
  mRadioState = (CC1120State)status.fields.STATE;

  // XXX(colin): fix later
  //return mRadioState;
  return (CC1120State)status.value;
}

int CC1120::sleep() {
  if (this->getState() != CC1120_STATE_IDLE) {
    CC1120Errno = BAD_STATE;
    return -1;
  }

  this->strobe(CC112X_SXOFF);

  CC1120Errno = OK;
  return 0;
}

int CC1120::deepSleep() {
  if (this->getState() != CC1120_STATE_IDLE) {
    CC1120Errno = BAD_STATE;
    return -1;
  }

  this->strobe(CC112X_SPWD);

  CC1120Errno = OK;
  return 0;
}

int CC1120::calibrate() {
  if (this->getState() != CC1120_STATE_IDLE) {
    CC1120Errno = BAD_STATE;
    return -1;
  }

  this->strobe(CC112X_SCAL);

  CC1120Errno = OK;
  return 0;
}

int CC1120::enableFastTransmit() {
  // update the state - will wake up from sleep
  this->getState();
  if (mRadioState != CC1120_STATE_IDLE) {
    CC1120Errno = BAD_STATE;
    return -1;
  }

  this->strobe(CC112X_SFSTXON);

  CC1120Errno = OK;
  return 0;
}

int CC1120::pushTxFifo(const char *data, uint8_t nBytes) {
  this->regAccess(RADIO_WRITE_ACCESS, CC112X_BURST_TXFIFO, &nBytes, 1);
  this->regAccess(RADIO_WRITE_ACCESS, CC112X_BURST_TXFIFO, (uint8_t*)data, nBytes);

  CC1120Errno = OK;
  return nBytes;
}

int CC1120::strobeTransmit() {
  switch(this->getState()) {
  case CC1120_STATE_CALIBRATE:
    while(this->getState() == CC1120_STATE_CALIBRATE) {
      wait(0.01);
    }
    break;
  case CC1120_STATE_SETTLING:
    while(this->getState() == CC1120_STATE_SETTLING) {
      wait(0.01);
    }
    break;
  case CC1120_STATE_FSTXON:
  case CC1120_STATE_IDLE:
    break;
  default:
    CC1120Errno = BAD_STATE;
    return -1;
  }

  this->strobe(CC112X_STX);

  CC1120Errno = OK;
  return 0;
}

int CC1120::popRxFifo(char *data, uint8_t nBytes) {
  // get the packet length first
  uint8_t packetLen;
  int res = this->regAccess(RADIO_READ_ACCESS, CC112X_BURST_RXFIFO, &packetLen, 1);
  if (res < 0) {
    return res;
  }

  if (packetLen <= nBytes) {
    res = this->regAccess(RADIO_READ_ACCESS, CC112X_BURST_RXFIFO, (uint8_t*)data, packetLen);
    return packetLen;
  } else {
    this->flushRxFifo();
    CC1120Errno = BAD_BUFFER;
    return -1;
  }
}

int CC1120::strobeReceive() {
  switch(this->getState()) {
  case CC1120_STATE_CALIBRATE:
    while(this->getState() == CC1120_STATE_CALIBRATE) {
      wait(0.01);
    }
    break;
  case CC1120_STATE_SETTLING:
    while(this->getState() == CC1120_STATE_SETTLING) {
      wait(0.01);
    }
    break;
  case CC1120_STATE_FSTXON:
  case CC1120_STATE_IDLE:
    break;
  default:
    CC1120Errno = BAD_STATE;
    return -1;
  }

  this->strobe(CC112X_SRX);

  CC1120Errno = OK;
  return 0;
}

int CC1120::setRxOffMode(int val) {
  uint8_t rfEndCfg1;
  this->getRegister(CC112X_RFEND_CFG1, &rfEndCfg1);

  // mask out the previous configuration then OR set again
  rfEndCfg1 &= 0xDF;
  rfEndCfg1 |= val << 4;

  this->setRegister(CC112X_RFEND_CFG1, rfEndCfg1);

  CC1120Errno = OK;
  return 0;
}

int CC1120::setTxOffMode(int val) {
  uint8_t rfEndCfg0;
  this->getRegister(CC112X_RFEND_CFG0, &rfEndCfg0);

  // mask out the previous configuration then OR set again
  rfEndCfg0 &= 0xDF;
  rfEndCfg0 |= val << 4;

  this->setRegister(CC112X_RFEND_CFG0, rfEndCfg0);

  CC1120Errno = OK;
  return 0;
}
