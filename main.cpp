#include <mbed.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

//#include <SDFileSystem.h>

DigitalOut myLed(LED1);

// sd(MOSI, MISO, SCLK, CS)
//SDFileSystem sd(D11, D12, D13, D10, "sd");

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

struct cc1120_status {
  uint8_t reserved : 4;
  uint8_t STATE : 3;
  uint8_t CHIP_RDY : 1;
};
union cc_st {
  struct cc1120_status status;
  uint8_t value;
};
union cc_st status;

// misc bits
#define RADIO_READ_BIT  0x80

// registers
#define FREQ_0            0x0e
#define EXTENDED_ADDRESS  0x2f

// strobes
#define SNOP    0x3d
#define SFSTXON 0x31
#define SXOFF   0x32

// extended registers
#define PARTNUMBER  0x8f
#define PARTVERSION 0X90

void strobe(uint8_t strobe);
uint8_t transferByte(uint8_t cmd_address, uint8_t value);
uint8_t writeRegister(uint8_t address, uint8_t value);
uint8_t readRegister(uint8_t address);
uint8_t readExtendedRegister(uint8_t extended_address);

int main() {
  printf("\r\n\n\n== START ==\r\n");

  // set reset and cs HI
  radioReset = 1;
  radioSpiCs = 1;

  // configure SPI for radio communication
  radioSpi.format(8, 0);
  radioSpi.frequency(4000000);

  while(1) {
    strobe(SNOP);

    printf("cc1120_status.CHIP_RDY: %d\r\n", status.status.CHIP_RDY);
    printf("cc1120_status.STATE: %d\r\n", status.status.STATE);

    // read the part number and version 
    uint8_t val = readExtendedRegister(PARTNUMBER);
    printf("Part Number: 0x%08x\r\n", (uint32_t)val);
    val = readExtendedRegister(PARTVERSION);
    printf("Part Version: 0x%08x\r\n", (uint32_t)val);

    wait(0.5);
  }

  /*
  while(1) {


    // sleep for 500ms
    wait(0.5);
  }
  */
}

void strobe ( uint8_t strobe ){
  radioSpiCs = 0;
  status.value = radioSpi.write(RADIO_READ_BIT | strobe);
  radioSpiCs = 1;
}

uint8_t transferByte(uint8_t cmd_address, uint8_t value) {
  uint8_t out;

  radioSpiCs = 0;
  status.value = radioSpi.write(cmd_address);
  out = radioSpi.write(value);
  radioSpiCs = 1;

  return out;
}
uint8_t writeRegister(uint8_t address, uint8_t value) {
  return transferByte( address, value );
}

uint8_t readRegister(uint8_t address) {
  return transferByte( RADIO_READ_BIT | address, 0x00 );
}

uint8_t readExtendedRegister(uint8_t extended_address) {
  uint8_t out;

  radioSpiCs = 0;
  status.value = radioSpi.write(RADIO_READ_BIT | EXTENDED_ADDRESS);
  radioSpi.write(extended_address);
  out = radioSpi.write(0xff);

  radioSpiCs = 1;

  return out;
}

extern "C" {
  int _kill(int pid, int sig) {
    sig = pid == pid;
    pid = sig == sig;
    errno = EINVAL;
    return -1;
  }

  int _getpid(void) {
    return 1;
  }
}
