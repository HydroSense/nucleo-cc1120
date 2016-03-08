#ifndef __MBED_H
#define __MBED_H

#include "MockDigitalOut.hpp"
#include "MockSPI.hpp"

#include <gmock/gmock.h>

/*
class MbedInterface {
public:
  virtual ~MbedInterface;
  virtual void wait(float) = 0;
}

class FakeMbed : public MbedInterface {
public:
  MOCK_METHOD1(wait, void float());
}
*/

// fake wait
void wait(float time);

#endif
