#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../mock/mbed.h"
#include "../../CC1120.hpp"

#include <string.h>

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::_;
using ::testing::Sequence;

class CC1120Test : public ::testing::Test {
protected:
  SPI mockSpi;
  DigitalOut mockSpiCs;
  DigitalOut mockReset;

  CC1120* radio;

  cc1120_status idleStatus;
  cc1120_status transmitStatus;

  CC1120Test() {
    radio = NULL;

    idleStatus.fields.CHIP_RDY = 0;
    idleStatus.fields.reserved = 0;
    idleStatus.fields.STATE = CC1120_STATE_IDLE;

    transmitStatus.fields.CHIP_RDY = 0;
    transmitStatus.fields.reserved = 0;
    transmitStatus.fields.STATE = CC1120_STATE_TRANSMIT;
  }

  virtual ~CC1120Test() {
  }

  virtual void SetUp() {
    EXPECT_CALL(mockReset, write(0))
      .Times(1);
    EXPECT_CALL(mockSpiCs, write(1))
      .Times(1);

    radio = new CC1120(mockSpi, mockSpiCs, mockReset);
  }

  virtual void TearDown() {
    delete radio;
  }
};

TEST_F(CC1120Test, begin) {
  EXPECT_CALL(mockReset, write(1))
    .Times(1);
  EXPECT_CALL(mockSpiCs, write(_))
    .Times(6);
  EXPECT_CALL(mockSpi, format(_,_))             // sets the SPI format
    .Times(1);
  EXPECT_CALL(mockSpi, frequency(_))            // sets the frequency
    .Times(1);
  EXPECT_CALL(mockSpi, write(CC112X_SFRX))      // flushes RX buffer
    .Times(1);
  EXPECT_CALL(mockSpi, write(CC112X_SFTX))      // flushes TX buffer
    .Times(1);
  EXPECT_CALL(mockSpi, write(CC112X_SNOP))      // strobes SNOP to query status
    .WillOnce(Return(idleStatus.value));

  radio->begin();
}

TEST_F(CC1120Test, end) {
  EXPECT_CALL(mockReset, write(0))
    .Times(1);

  radio->end();
}

TEST_F(CC1120Test, setRegisterPlain) {
  uint8_t val = 17;
  uint16_t reg = CC112X_IOCFG3;
  EXPECT_CALL(mockSpi, write(RADIO_WRITE_ACCESS|RADIO_BURST_ACCESS|(uint8_t)reg))
    .Times(1);
  EXPECT_CALL(mockSpi, write(val))
    .Times(1);

  EXPECT_CALL(mockSpiCs, write(_))
    .Times(2);

  int res = radio->setRegister(reg, val);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(CC1120Errno, OK);
}

TEST_F(CC1120Test, setRegisterExtended) {
  uint8_t val = 0x17;
  uint16_t reg = CC112X_WOR_TIME1;
  EXPECT_CALL(mockSpi, write(RADIO_WRITE_ACCESS|RADIO_BURST_ACCESS|0x2F))
    .Times(1);
  EXPECT_CALL(mockSpi, write(reg & 0x00FF))
    .Times(1);
  EXPECT_CALL(mockSpi, write(val))                // expect we write the value
    .Times(1);

  EXPECT_CALL(mockSpiCs, write(_))                // expect single transmission period
    .Times(2);

  int res = radio->setRegister(reg, val);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(CC1120Errno, OK);
}

TEST_F(CC1120Test, setRegisterInvalid) {
  uint8_t val = 0x17;
  uint16_t reg = 0xFFFF;
  EXPECT_CALL(mockSpi, write(_))
    .Times(0);
  EXPECT_CALL(mockSpiCs, write(_))
    .Times(0);

  int res = radio->setRegister(reg, val);
  ASSERT_NE(res, 0);
  ASSERT_EQ(CC1120Errno, BAD_REG_ACCESS);
}

TEST_F(CC1120Test, getRegisterPlain) {
  uint8_t val;
  uint8_t expected_val = 0x17;
  uint16_t reg = CC112X_AGC_CFG3;

  EXPECT_CALL(mockSpi, write(RADIO_READ_ACCESS|RADIO_BURST_ACCESS|(uint8_t)reg))
    .WillOnce(Return(idleStatus.value));
  EXPECT_CALL(mockSpi, write(0x00))
    .WillOnce(Return(expected_val));
  EXPECT_CALL(mockSpiCs, write(_))
    .Times(2);

  int res = radio->getRegister(reg, &val);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(CC1120Errno, OK);
  ASSERT_EQ(val, expected_val);
}

TEST_F(CC1120Test, getRegisterExtended) {
  uint8_t val;
  uint8_t expected_val = 0x17;
  uint16_t reg = CC112X_WOR_TIME1;

  EXPECT_CALL(mockSpi, write(RADIO_READ_ACCESS|RADIO_BURST_ACCESS|0x2F))   // extended declaration
    .Times(1);
  EXPECT_CALL(mockSpi, write(reg & 0x00FF))   // the register value
    .Times(1);
  EXPECT_CALL(mockSpi, write(0x00))           // write one byte to get reg value out
    .WillOnce(Return(expected_val));

  EXPECT_CALL(mockSpiCs, write(_))            // expect the CS is asserted during transaction
    .Times(2);

  int res = radio->getRegister(reg, &val);
  ASSERT_EQ(res, 0);
  ASSERT_EQ(CC1120Errno, OK);
  ASSERT_EQ(val, expected_val);
}

TEST_F(CC1120Test, getRegisterInvalid) {
  uint8_t val = 0x78;
  uint8_t expected_val = val;
  uint16_t reg = 0xEEEE;

  // ensure the SPI interface is not being used
  EXPECT_CALL(mockSpi, write(_))
    .Times(0);
  EXPECT_CALL(mockSpiCs, write(_))
    .Times(0);

  int res = radio->getRegister(reg, &val);
  ASSERT_NE(res, 0);
  ASSERT_EQ(CC1120Errno, BAD_REG_ACCESS);
  ASSERT_EQ(val, expected_val);
}

TEST_F(CC1120Test, getState) {
  CC1120State state;
  CC1120State expectedState = CC1120_STATE_TRANSMIT;

  EXPECT_CALL(mockSpi, write(CC112X_SNOP))    // expect strobing the SNOP to get value
    .WillOnce(Return(expectedState));
  EXPECT_CALL(mockSpiCs, write(_))            // expect CS assertion
    .Times(2);

  state = radio->getState();
  ASSERT_EQ(state, expectedState);
}

TEST_F(CC1120Test, sleep) {
  EXPECT_CALL(mockSpi, write(CC112X_SNOP))      // for getting state
    .Times(1);
  EXPECT_CALL(mockSpi, write(CC112X_SXOFF))     // strobing the off
    .Times(1);
  EXPECT_CALL(mockSpiCs, write(_))              // ensure CS is called at least twice
    .Times(AtLeast(2));

  int res = radio->sleep();
  ASSERT_EQ(res, 0);
  ASSERT_EQ(CC1120Errno, OK);
}

TEST_F(CC1120Test, sleepInvalidState) {
  EXPECT_CALL(mockSpi, write(CC112X_SNOP))      // for getting state
    .WillOnce(Return(CC1120_STATE_TRANSMIT));
  EXPECT_CALL(mockSpiCs, write(_))              // ensure CS is called at least twice
    .Times(AtLeast(2));

  int res = radio->sleep();
  ASSERT_NE(res, 0);
  ASSERT_EQ(CC1120Errno, BAD_STATE);
}

TEST_F(CC1120Test, deepSleep) {
  EXPECT_CALL(mockSpi, write(CC112X_SNOP))      // for getting state
    .Times(1);
  EXPECT_CALL(mockSpi, write(CC112X_SPWD))      // strobing the off
    .Times(1);
  EXPECT_CALL(mockSpiCs, write(_))              // ensure CS is called at least twice
    .Times(AtLeast(2));

  int res = radio->deepSleep();
  ASSERT_EQ(res, 0);
  ASSERT_EQ(CC1120Errno, OK);
}

TEST_F(CC1120Test, deepSleepInvalidState) {
  EXPECT_CALL(mockSpi, write(CC112X_SNOP))      // for getting state
    .WillOnce(Return(CC1120_STATE_TRANSMIT));
  EXPECT_CALL(mockSpiCs, write(_))              // ensure CS is called at least twice
    .Times(AtLeast(2));

  int res = radio->deepSleep();
  ASSERT_NE(res, 0);
  ASSERT_EQ(CC1120Errno, BAD_STATE);
}