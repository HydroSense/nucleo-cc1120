#include <mbed.h>

#include "CC1120.hpp"

// pins used for CC1120 radio communication
#define RADIO_SPI_MOSI  PB_15
#define RADIO_SPI_MISO  PB_14
#define RADIO_SPI_SCLK  PB_13
#define RADIO_SPI_CS    PB_1

#define RADIO_RESET PB_10
#define RADIO_GPIO_0 PB_5
#define RADIO_GPIO_2 PB_4
#define RADIO_GPIO_3 PA_8

CC1120 radio(RADIO_SPI_MOSI, RADIO_SPI_MISO, RADIO_SPI_SCLK, RADIO_SPI_CS, RADIO_RESET);

void configureRadio(){
  #include "CC1120.prs"
}

int main() {
  printf("\r\n\n\n== START ==\r\n");

  // start the radio interface and then configure
  radio.begin();
  configureRadio();

  // wait 0.1s before printing info to the console
  wait(0.1);
  uint8_t val;
  int res = radio.getRegister(CC112X_PARTNUMBER, &val);
  printf("Part Number: 0x%08x\r\n", (unsigned int)val);
  radio.getRegister(CC112X_PARTVERSION, &val);
  printf("Part Version: 0x%08x\r\n", (unsigned int)val);

  // set the RX off mode to go into the idle state
  radio.setRxOffMode(0x00);

  // enter loop
  char data[16] = {'\0'};
  while(1) {
    printf("Top of loop...\r\n");

    radio.strobeReceive();
    while(radio.getState() != CC1120_STATE_IDLE) {
      wait(0.01);
    }

    int res = radio.popRxFifo(data, 16);
    data[res] = '\0';
    printf("data: %s\r\n", data);

    wait(0.5);
  }
}
