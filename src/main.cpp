#include "QEI.hpp"
#include "amt21.hpp"
#include "mbed.h"

BufferedSerial pc{USBTX, USBRX, 115200};

DigitalIn user(BUTTON1);

Timer timer;

bool push = false;

int main() {
  Amt21 amt({0x50, 0x58}, PB_6, PA_10, (int)2e6, PC_0);  // node_addresses, tx, rx, baud, de
  QEI encoder(PA_0, PA_1, QEI::X2_ENCODING);

  printf("\nsetup\n");
  timer.start();
  auto pre = timer.elapsed_time();
  while (1) {
    auto now = timer.elapsed_time();
    int now_pulses = encoder.getPulses();
    if (now - pre > 10ms) {
      amt.request_all_pos();
      printf("abs:%ld %ld", amt.pos[0x50], amt.pos[0x58]);
      printf("rori:%ld \n", now_pulses);
      pre = now;
    }

    if (!user && !push) {
      amt.request_zero(0x58);
      amt.request_zero(0x50);
      push = true;
    } else if (user) {
      push = false;
    }
  }
}