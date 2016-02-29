#include <mbed.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "hal_types.h"
#include "cc112x_spi.h"

//#include <SDFileSystem.h>

DigitalOut myLed(LED1);

// sd(MOSI, MISO, SCLK, CS)
//SDFileSystem sd(D11, D12, D13, D10, "sd");

void configureRadio() {
  #include "CC1120.prs"
}

int main() {
  printf("\r\n\n\n== START ==\r\n");

  // initialize the SPI connection
  trxSpiInit();
  configureRadio();

  while(1) {
    union cc1120_status status;
    status.value = trxSpiCmdStrobe(CC112X_SNOP);

    printf("cc1120_status.CHIP_RDY: %d\r\n", status.fields.CHIP_RDY);
    printf("cc1120_status.STATE: %d\r\n", status.fields.STATE);

    // read the part number and version
    uint8_t val;
    cc112xSpiReadReg(CC112X_PARTNUMBER, &val, 1);
    printf("Part Number: 0x%08x\r\n", (unsigned int)val);
    cc112xSpiReadReg(CC112X_PARTVERSION, &val, 1);
    printf("Part Version: 0x%08x\r\n", (unsigned int)val);

    wait(0.5);
  }

  /*
  while(1) {


    // sleep for 500ms
    wait(0.5);
  }
  */
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
