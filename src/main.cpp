#include "AMT21.hpp"
#include "QEI.hpp"
#include "mbed.h"

BufferedSerial pc{USBTX, USBRX, 115200};
DigitalIn user(BUTTON1);

Timer timer;

bool push = false;

AMT21 amt(PB_6, PA_10, (int)2e6, PC_0);
QEI encoder(PA_0, PA_1, QEI::X2_ENCODING);

std::vector<uint8_t> addresses;

void address_srch() {
  printf("\nsrch\n");
  while (true) {
    for (int row = 0; row < 8; ++row) {        // 下位 nibble (0x0, 0x4, 0x8, ..., 0x1C)
      for (int col = 0; col < 16; col += 2) {  // 上位 nibble (0x0, 0x2, 0x4, ..., 0xE)
        uint8_t address = (row * 4) | (col << 4);
        if (amt.request_pos(address) && amt.request_turn(address)) {
          if (std::find(addresses.begin(), addresses.end(), address) == addresses.end())
            addresses.push_back(address);
        } else if (std::find(addresses.begin(), addresses.end(), address) != addresses.end()) {
          addresses.erase(std::remove(addresses.begin(), addresses.end(), address), addresses.end());
        }
        ThisThread::sleep_for(50ms);
      }
    }
    ThisThread::sleep_for(100ms);
  }
}

int main() {
  std::map<uint8_t, int32_t> real_pos;  // pos + turn*rotate
  std::map<uint8_t, int32_t> offset;
  Thread thread;
  thread.start(address_srch);
  timer.start();
  auto pre = timer.elapsed_time();
  printf("\nsetup\n");
  while (true) {
    auto now = timer.elapsed_time();
    if (now - pre > 10ms) {
      amt.request_all(addresses);
      for (uint8_t address : addresses) {
        real_pos[address] = amt.pos_[address] + amt.turn_[address] * AMT21::rotate;
        printf("[0x%x]: %ld ", address, real_pos[address]);
      }
      printf("\n");
      pre = now;
    }
  }
  return 0;
}