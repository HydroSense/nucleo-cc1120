#include <mbed.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include <SDFileSystem.h>

DigitalOut myLed(LED1);

// sd(MOSI, MISO, SCLK, CS)
SDFileSystem sd(D11, D12, D13, D10, "sd");

int main() {
  printf("\r\n\r\n\r\n=== Start ===\r\n");
  FILE* file = fopen("/sd/data.txt", "w");
  fprintf(file, "Hello World!\n");
  fclose(file);

  while(1) {
    myLed = 1;
    wait(0.5);
    myLed = 0;
    wait(1.0);

    printf("Hello World!\r\n");
  }
}

extern "C" {
  int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
  }

  int _getpid(void) {
    return 1;
  }
}
