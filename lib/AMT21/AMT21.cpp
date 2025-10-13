#include "AMT21.hpp"

AMT21::AMT21(const PinName tx, const PinName rx, const int baud, const PinName de) : rs485_(tx, rx, baud, de) {}

bool AMT21::request_pos(const uint8_t address) {
  rs485_.uart_transmit({address});  // request postion -> <node_address>
  if (uint16_t now_pos; rs485_.uart_receive(&now_pos, sizeof(now_pos), 5ms) && is_valid(now_pos)) {
    pos_[address] = (now_pos & 0x3fff) >> 2;  // 12bit
    return true;
  }
  return false;
}

bool AMT21::request_turn(const uint8_t address) {
  rs485_.uart_transmit({uint8_t(address + 1)});
  if (uint16_t now_turn; rs485_.uart_receive(&now_turn, sizeof(now_turn), 5ms) && is_valid(now_turn)) {
    now_turn = (now_turn & 0x3fff);  // 14bit
    uint16_t raw = now_turn;
    int turn = (raw & 0x2000) == 0 ? raw : -((~raw & 0x1fff) + 1);
    turn_[address] = turn;
    return true;
  }
  return false;
}

void AMT21::request_all(std::vector<uint8_t> addresses) {
  for (uint8_t address : addresses) {
    request_pos(address);
    request_turn(address);
  }
}

void AMT21::request_reset(const uint8_t address) {
  rs485_.uart_transmit({address + 2, 0x75});  // encoder reset -> <node_address + 0x02> <0x75>
}

bool AMT21::is_valid(uint16_t raw_data) {
  bool k1 = raw_data >> 15;
  bool k0 = raw_data >> 14 & 1;
  raw_data <<= 2;
  do {
    k1 ^= raw_data & 0x8000;          // even
    k0 ^= (raw_data <<= 1) & 0x8000;  // odd
  } while (raw_data <<= 1);
  return k0 && k1;
}
