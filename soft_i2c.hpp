#ifndef SOFT_I2C_HPP_INCLUDED
#define SOFT_I2C_HPP_INCLUDED

/**
  @file
  @author Alexander Panasyuk
  Bit-banged (software) I2C master on two general-purpose pins. Use it where the pins an I2C
  slave is wired to can't drive a hardware I2C peripheral (e.g. an I2C-only panel routed to a
  UART's pins). The pins must be open-drain with pull-ups (external, on the slave/bus): a line
  is released high by writing 1 and driven low by writing 0, and is read back via its input
  register. Clock-stretching by the slave is honored.

  Pins are passed as @c avp::Pin types, so:
  @code
    typedef avp::SoftI2C<AVP_PIN(A,9), AVP_PIN(A,10)> MyBus; // SDA=PA9, SCL=PA10
    MyBus::Init();
    MyBus::Write(0xEE, cmd, n);   // 8-bit address (R/W bit included)
  @endcode
  */

/// @cond
#include <stdint.h>
/// @endcond
#include "gpio.hpp" // avp::Pin, AVP_PIN

namespace avp {

  /// @tparam SDA,SCL  @c avp::Pin types for the data and clock lines
  /// @tparam DelayCount  busy-loop iterations per I2C half-bit; tune for the desired bus speed
  template<class SDA, class SCL, uint32_t DelayCount = 120>
  class SoftI2C {
    static inline void Dly() { for(volatile uint32_t i = 0; i < DelayCount; ++i) {} }
    /// release SCL high and wait out any slave clock-stretch (bounded so a stuck bus can't hang)
    static inline void SclHigh() {
      SCL::set_high();
      for(uint32_t g = 0; !SCL::get() && g < 10000; ++g) {}
    }

   public:
    static void Init() {
      GPIO_InitTypeDef g = {}; // designated fields: F1's GPIO_InitTypeDef has no Alternate member
      g.Mode = GPIO_MODE_OUTPUT_OD; g.Pull = GPIO_PULLUP; g.Speed = GPIO_SPEED_FREQ_LOW;
      g.Pin = SDA::PinMask; HAL_GPIO_Init((GPIO_TypeDef *)SDA::Port, &g);
      g.Pin = SCL::PinMask; HAL_GPIO_Init((GPIO_TypeDef *)SCL::Port, &g);
      SDA::set_high(); SCL::set_high();
    }

    static void Start() { SDA::set_high(); SclHigh(); Dly(); SDA::set_low(); Dly(); SCL::set_low(); Dly(); }
    static void Stop()  { SDA::set_low(); Dly(); SclHigh(); Dly(); SDA::set_high(); Dly(); }

    /// @return true if the slave ACKed
    static bool WriteByte(uint8_t b) {
      for(uint8_t i = 0; i < 8; ++i) { SDA::set(b & 0x80); b <<= 1; Dly(); SclHigh(); Dly(); SCL::set_low(); }
      SDA::set_high(); Dly(); SclHigh(); Dly(); bool ack = !SDA::get(); SCL::set_low(); Dly(); return ack;
    }
    /// @param ack  master drives SDA low (ACK) for all but the last byte of a read; false = NACK
    static uint8_t ReadByte(bool ack) {
      uint8_t b = 0; SDA::set_high();
      for(uint8_t i = 0; i < 8; ++i) { Dly(); SclHigh(); b = (b << 1) | (SDA::get() ? 1 : 0); Dly(); SCL::set_low(); }
      SDA::set(!ack); Dly(); SclHigh(); Dly(); SCL::set_low(); SDA::set_high(); Dly(); return b;
    }

    /// Standard write transaction. @param addr8 8-bit address with R/W bit = 0
    static bool Write(uint8_t addr8, const uint8_t *p, uint8_t n) {
      Start();
      bool ok = WriteByte(addr8);
      for(uint8_t i = 0; i < n && ok; ++i) ok = WriteByte(p[i]);
      Stop();
      return ok;
    }
    /// Standard read transaction (ACK all but the last byte). @param addr8 8-bit address, R/W bit = 1
    static bool Read(uint8_t addr8, uint8_t *p, uint8_t n) {
      Start();
      bool ok = WriteByte(addr8);
      for(uint8_t i = 0; ok && i < n; ++i) p[i] = ReadByte(i + 1 < n);
      Stop();
      return ok;
    }
  }; // SoftI2C

} // namespace avp

#endif /* SOFT_I2C_HPP_INCLUDED */
