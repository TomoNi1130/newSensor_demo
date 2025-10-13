#ifndef AMT21_H_
#define AMT21_H_

#include <map>
#include <vector>

#include "Rs485.h"
#include "mbed.h"

class AMT21 {
 public:
  AMT21(const PinName tx, const PinName rx, const int baud, const PinName de);
  void request_reset(const uint8_t address);
  bool request_pos(const uint8_t address);
  bool request_turn(const uint8_t address);
  void request_all(std::vector<uint8_t> addresses);

  std::map<uint8_t, int32_t> pos_;   // address:pos
  std::map<uint8_t, int32_t> turn_;  // address:turn num

  static constexpr int rotate = 4096;  // 12bit

 private:
  bool is_valid(uint16_t raw_data);

  Rs485 rs485_;
};

#endif