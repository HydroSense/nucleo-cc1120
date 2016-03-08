#ifndef __MOCK_DIGITAL_OUT_HPP
#define __MOCK_DIGITAL_OUT_HPP

#include <sys/types.h>
#include <gmock/gmock.h>

class DigitalOutInterface {
public:
  virtual ~DigitalOutInterface() {};
  virtual void write(int) = 0;
  virtual int read(void) = 0;
};

class DigitalOut : public DigitalOutInterface {
public:
  MOCK_METHOD1(write, void(int));
  MOCK_METHOD0(read, int(void));
};

#endif
