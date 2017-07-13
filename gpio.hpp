#ifndef GPIO_HPP_INCLUDED
#define GPIO_HPP_INCLUDED

#include <stdint.h>
#include <stm32f3xx.h>
#include <stm32f3xx_hal_gpio.h>
#include <AVP_LIBS/General/Macros.h>

namespace avp {
  enum PinMode {INPUT, OUTPUT};
  enum PinLevel {LOW, HIGH};

#if defined(AVP_STATIC_PIN) && AVP_STATIC_PIN

  template<uint32_t GPIOx, uint16_t GPIO_Pin>
  struct Pin {
    /*! sets PULLUP when mode is INPUT */
    /* CubeMX does it by itself */
    static void Config(uint32_t Mode = GPIO_MODE_INPUT, uint32_t Pull = GPIO_NOPULL,
                       uint32_t Speed = GPIO_SPEED_LOW, uint32_t Alternate = GPIO_AF15_EVENTOUT) {
      GPIO_InitTypeDef GPIO_Init = {GPIO_Pin, Mode, Pull, Speed, Alternate};
      HAL_GPIO_Init((GPIO_TypeDef *)GPIOx,&GPIO_Init);
    } // Init
    // static constexpr char *Name() { return (const char *)PortName; }
    static void write(bool value) { HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOx, GPIO_Pin, value?GPIO_PIN_SET:GPIO_PIN_RESET); }
    static void set_high() { write(true); }
    static void set_low() { write(false); }
    static bool read() { return HAL_GPIO_ReadPin((GPIO_TypeDef *)GPIOx, GPIO_Pin) != GPIO_PIN_RESET; }
    static void toggle() { HAL_GPIO_TogglePin((GPIO_TypeDef *)GPIOx, GPIO_Pin); }
  }; // Pin

#define AVP_PIN(Port,Number)  avp::Pin<uint32_t(COMB2(GPIO,Port)),COMB2(GPIO_PIN_,Number)>

#else
  /// Pin base class
  class Pin {
    uint32_t GPIOx;
    uint16_t GPIO_Pin;

    Pin(uint32_t GPIOx_, uint16_t GPIO_Pin_uint32_t Mode = GPIO_MODE_INPUT, uint32_t Pull = GPIO_NOPULL,
        uint32_t Speed = GPIO_SPEED_LOW, uint32_t Alternate = GPIO_AF15_EVENTOUT):GPIOx(GPIOx_), GPIO_Pin(GPIO_Pin_) {
     GPIO_InitTypeDef GPIO_Init = {GPIO_Pin, Mode, Pull, Speed, Alternate};
      HAL_GPIO_Init((GPIO_TypeDef *)GPIOx,&GPIO_Init);
    } // constructor


  }; // Pin_


#endif

#if 0

  namespace avp {
    typedef struct  {
      GPIO_TypeDef *GPIOx;
      uint16_t GPIO_Pin;
    } GPIOpin;
#define AVP_DEFINE_PIN(Name,Port,Number) static constexpr avp::GPIOpin Name = {_COMB2(GPIO,Port),_COMB2(GPIO_PIN_,Number)};
#endif

  } // namespace avp

#endif /* GPIO_HPP_INCLUDED */

