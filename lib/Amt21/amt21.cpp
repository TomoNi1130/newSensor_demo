#include "amt21.hpp"

Amt21::Amt21(const std::vector<uint8_t> init_addresses, const PinName tx, const PinName rx, const int baud, const PinName de) : addresses_(init_addresses), rs485(tx, rx, baud, de) {
  for (uint8_t address : init_addresses) {
    pos[address] = 0;
    pre_pos[address] = 0;
    offset[address] = 0;
  }
}

bool Amt21::request_pos(const uint8_t address) {
  rs485.uart_transmit({address});  // request postion -> <node_address>
  if (uint16_t now_pos; rs485.uart_receive(&now_pos, sizeof(now_pos), 10ms) && is_valid(now_pos)) {
    now_pos = (now_pos & 0x3fff) >> 2;  // 位置を2ビット右にシフト（12bitのため）
    now_pos -= offset[address];
    int16_t diff = now_pos - pre_pos[address];
    if (diff > rotate / 2) {
      diff -= rotate;
    } else if (diff < -rotate / 2) {
      diff += rotate;
    }
    pos[address] += diff;
    pre_pos[address] = now_pos;
    return true;
  }
  return false;
}

bool Amt21::request_all_pos() {
  bool all_ok = true;
  for (uint8_t address : addresses_)
    if (!request_pos(address)) all_ok = false;
  return all_ok;
}

void Amt21::request_reset(const uint8_t address) {
  rs485.uart_transmit({address + 2, 0x75});  // encoder reset -> <node_address + 0x02> <0x75>
}

void Amt21::request_zero(const uint8_t address) {
  offset[address] += pos[address];
}