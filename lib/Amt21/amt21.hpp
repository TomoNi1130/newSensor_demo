#ifndef AMT21_H_
#define AMT21_H_

#include <map>
#include <vector>

#include "Rs485.h"
#include "mbed.h"

class Amt21 {
 public:
  Amt21(const std::vector<uint8_t> init_addresses, const PinName tx, const PinName rx, const int baud, const PinName de);
  bool request_pos(const uint8_t address);
  bool request_all_pos();
  void request_reset(const uint8_t address);
  void request_zero(const uint8_t address);

  std::map<uint8_t, int32_t> pos;  // <address pos>

 private:
  static bool is_valid(uint16_t raw_data) {
    bool k1 = raw_data >> 15;
    bool k0 = raw_data >> 14 & 1;
    raw_data <<= 2;
    do {
      k1 ^= raw_data & 0x8000;          // even
      k0 ^= (raw_data <<= 1) & 0x8000;  // odd
    } while (raw_data <<= 1);
    return k0 && k1;
  }

  static constexpr int rotate = 4096;  // 12bit

  std::vector<uint8_t> addresses_;
  std::map<uint8_t, int32_t> pre_pos;
  std::map<uint8_t, int32_t> offset;

  Rs485 rs485;
};

#endif