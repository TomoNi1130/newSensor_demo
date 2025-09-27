#include "AMT21.hpp"
#include "QEI.hpp"
#include "mbed.h"

BufferedSerial pc{USBTX, USBRX, 115200};
DigitalIn user(BUTTON1);

Timer timer;

bool push = false;

int main() {
  AMT21 amt(PB_6, PA_10, (int)2e6, PC_0);
  QEI encoder(PA_0, PA_1, QEI::X2_ENCODING);
  timer.start();
  auto pre = timer.elapsed_time();
  printf("\nsetup\n");
  while (true) {
    auto now = timer.elapsed_time();
    if (now - pre > 10ms) {
      amt.process();
      int now_pulses = encoder.getPulses();
      printf("abs:");
      for (uint8_t address : amt.addresses_)
        printf("[0x%x]%d, ", address, amt.real_pos_[address]);
      printf("rori:%d \n", now_pulses);
      pre = now;
    }

    if (!user && !push) {
      amt.address_srch();
      push = true;
    } else if (user) {
      push = false;
    }
  }
  return 0;
}