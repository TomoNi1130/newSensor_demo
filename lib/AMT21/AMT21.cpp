#include "AMT21.hpp"

AMT21::AMT21(const PinName tx, const PinName rx, const int baud, const PinName de) : rs485_(tx, rx, baud, de) {
  address_srch();
  for (uint8_t address : addresses_) {
    pos_[address] = 0;
    turn_[address] = 0;
    offset_[address] = 0;
    maybeOff_[address] = false;
  }
}

void AMT21::process() {
  request_all();
  for (uint8_t address : addresses_) {
    real_pos_[address] = pos_[address] + turn_[address] * rotate;
  }
}

void AMT21::address_srch() {
  addresses_.clear();
  for (int row = 0; row < 8; ++row) {        // 下位 nibble (0x0, 0x4, 0x8, ..., 0x1C)
    for (int col = 0; col < 16; col += 2) {  // 上位 nibble (0x0, 0x2, 0x4, ..., 0xE)
      uint8_t address = (row * 4) | (col << 4);
      if (request_pos(address) && request_turn(address)) {
        addresses_.push_back(address);
      }
    }
  }
}

bool AMT21::request_pos(const uint8_t address) {
  static std::map<uint8_t, int> count;
  if (count[address] < -5) count[address] = -5;
  if (count[address] > 5) count[address] = 5;
  maybeOff_[address] = (count[address] > 0);
  rs485_.uart_transmit({address});  // request postion -> <node_address>
  if (uint16_t now_pos; rs485_.uart_receive(&now_pos, sizeof(now_pos), 5ms) && is_valid(now_pos)) {
    pos_[address] = (now_pos & 0x3fff) >> 2;  // 12bit
    count[address]--;
    return true;
  }
  count[address]++;
  return false;
}

bool AMT21::request_turn(const uint8_t address) {
  static int pre_turn;
  rs485_.uart_transmit({uint8_t(address + 1)});
  if (uint16_t now_turn; rs485_.uart_receive(&now_turn, sizeof(now_turn), 5ms) && is_valid(now_turn)) {
    now_turn = (now_turn & 0x3fff);  // 14bit
    uint16_t raw = now_turn;
    int turn = (raw & 0x2000) == 0 ? raw : -((~raw & 0x1fff) + 1);
    if (!maybeOff_[address] && std::abs(turn - pre_turn) > 2)
      return false;
    if (offset_[address] != turn_[address] && turn == 0 && maybeOff_[address]) {
      offset_[address] = turn_[address];
    }
    turn_[address] = turn + offset_[address];
    return true;
  }
  return false;
}

void AMT21::request_all() {
  for (uint8_t address : addresses_) {
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
