#include "hal_types.h"
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

  mRadioSpiCs = 0;
  status = mRadioSpi.write(cmd);
  mRadioSpiCs = 1;

  return status;
}

int CC1120::regAccess(uint8_t accessType, uint16_t addr, uint8_t* pData, uint8_t len) {
  rfStatus_t status;

  uint8_t extAddr = addr >> 8;
  uint8_t tmpAddr = addr & 0x00FF;

  if (extAddr == 0x2F) {              // extended address case
    mRadioSpiCs = 0;

    status = mRadioSpi.write((accessType|RADIO_BURST_ACCESS) | extAddr);
    mRadioSpi.write(tmpAddr);

    this->readWriteBurst(accessType, pData, len);

    mRadioSpiCs = 1;
  } else if (extAddr == 0x00) {       // normal address case
    mRadioSpiCs = 0;

    status = mRadioSpi.write((accessType|RADIO_BURST_ACCESS) | tmpAddr);

    this->readWriteBurst(accessType, pData, len);

    mRadioSpiCs = 1;
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
CC1120::CC1120(PinName spiMosiPin, PinName spiMisoPin, PinName spiSClkPin, PinName spiCSPin, PinName resetPin) :
  mRadioSpi(spiMosiPin, spiMisoPin, spiSClkPin),
  mRadioSpiCs(spiCSPin),
  mRadioReset(resetPin) {

  // hold the radio reset high until we begin
  mRadioReset = 0;
  mRadioState = CC1120_STATE_UNINITIALIZED;
}

void CC1120::begin(void) {
  // deassert the CS while turning off reset
  mRadioSpiCs = 1;
  mRadioReset = 1;

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
  mRadioReset = 0;
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

  return mRadioState;
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

// TODO(colin): allow transmitting more data than the transmit buffer allows
int CC1120::sendSync(const char* data, uint8_t nBytes) {
  // wait in 10ms increments for the radio to finish transmitting
  // before continuing
  while(this->getState() == CC1120_STATE_TRANSMIT ||
        mRadioState == CC1120_STATE_SETTLING) {
    wait(0.001);
  }

  if (mRadioState != CC1120_STATE_IDLE &&
      mRadioState != CC1120_STATE_FSTXON) {
        CC1120Errno = BAD_STATE;
        return -1;
      }

  this->regAccess(0x00, CC112X_BURST_TXFIFO, (uint8_t*)data, nBytes);
  this->strobe(CC112X_STX);

  CC1120Errno = OK;
  return 0;
}

// TODO(colin): allow receiving more data than the receive buffer allows
int CC1120::recvSync(char* data, uint8_t nBytes) {
  printf("recv\r\n");

  // wait until the radio is out of calibration
  while (this->getState() == CC1120_STATE_CALIBRATE ||
         mRadioState == CC1120_STATE_SETTLING) {
    printf("1");
    wait(0.001);
  }

  if (mRadioState != CC1120_STATE_IDLE &&
      mRadioState != CC1120_STATE_FSTXON) {
        printf("failure");
        CC1120Errno = BAD_STATE;
        return -1;
      }

  // TODO(colin): investigate proper way to be in receive mode and allow time
  //              to collect packets

  printf("send SRX\r\n");
  this->strobe(CC112X_SRX);

  // wait until we have received a packet
  printf("wait for packet\r\n");
  while(this->getState() != CC1120_STATE_IDLE) {
    wait(0.1);
  }

  printf("getting length");
  this->regAccess(RADIO_READ_ACCESS, CC112X_BURST_RXFIFO, (uint8_t*)data, 1);
  printf("got length\r\n");
  int packetLen = (int)data[0] + 2;
  printf("len: %d\r\n", packetLen);
  printf("getting rest of packet\r\n");
  this->regAccess(RADIO_READ_ACCESS, CC112X_BURST_RXFIFO, (uint8_t*)data, packetLen);
  printf("done\r\n");

  CC1120Errno = OK;
  return packetLen;
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
  return this->regAccess(RADIO_WRITE_ACCESS, CC112X_BURST_TXFIFO, (uint8_t*)data, nBytes);
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
