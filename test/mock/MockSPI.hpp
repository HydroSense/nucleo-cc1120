#ifndef __MOCK_SPI_HPP
#define __MOCK_SPI_HPP

class SPIInterface {
public:
  virtual ~SPIInterface() {};
  virtual char write(char data) = 0;
  virtual void format(int, int) = 0;
  virtual void frequency(int) = 0;
};

#include <sys/types.h>
#include <gmock/gmock.h>

class SPI : public SPIInterface {
public:
  MOCK_METHOD1(write, char(char));
  MOCK_METHOD2(format, void(int, int));
  MOCK_METHOD1(frequency, void(int));
};

#endif
