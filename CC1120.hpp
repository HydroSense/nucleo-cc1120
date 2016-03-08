#ifndef __CC1120_HPP
#define __CC1120_HPP

#include <mbed.h>

#include "cc112x_spi.h"

struct CC1120Info {
  unsigned int partNumber;
  unsigned int partVersion;
};

enum CC1120State {
  CC1120_STATE_IDLE = 0,
  CC1120_STATE_RECEIVE = 1,
  CC1120_STATE_TRANSMIT = 2,
  CC1120_STATE_FSTXON = 3,
  CC1120_STATE_CALIBRATE = 4,
  CC1120_STATE_SETTLING = 5,
  CC1120_STATE_TX_FIFO_ERROR = 6,
  CC1120_STATE_RX_FIFO_ERROR = 7,
  CC1120_STATE_SLEEP,
  CC1120_STATE_CR_OSX_OFF,
  CC1120_STATE_UNINITIALIZED
};

enum CC1120Error {
  OK,
  BAD_STATE,
  BAD_REG_ACCESS,
  BAD_PARAMETER,
  BAD_BUFFER
};
extern CC1120Error CC1120Errno;

class CC1120 {
private:
  SPI& mRadioSpi;
  DigitalOut& mRadioSpiCs;
  DigitalOut& mRadioReset;

  CC1120State mRadioState;

  void flushTxFifo(void);
  void flushRxFifo(void);
  rfStatus_t strobe(uint8_t stb);

  int regAccess(uint8_t accessType, uint16_t addr, uint8_t* pData, uint8_t len);
  void readWriteBurst(uint8_t type, uint8_t* pData, uint8_t len);

public:
  //CC1120(PinName spiMosiPin, PinName spiMisoPin, PinName spiSClkPin, PinName spiCSPin, PinName resetPin);
  CC1120(SPI& radioSpi, DigitalOut& spiCsPin, DigitalOut& resetPin);

  // starting and ending associated interfaces
  void begin(void);
  void end(void);

  // used for configuration
  int setRegister(uint16_t reg, uint8_t val);
  int getRegister(uint16_t reg, uint8_t* val);

  // etc. state transitions (from idle)
  CC1120State getState(void);
  int sleep(void);
  int deepSleep(void);
  int calibrate(void);

  // asynchronous packet transmission
  int enableFastTransmit(void);
  int pushTxFifo(const char* data, uint8_t nBytes);
  int strobeTransmit(void);
  int popRxFifo(char* data, uint8_t nBytes);
  int strobeReceive(void);
  //int setReceiveTimeout(int time);        // see section 9.5 in documentation

  // transmit/receive configuration
  int setRxOffMode(int val);
  int setTxOffMode(int val);
};

#endif
