#ifndef GPIO_HPP_INCLUDED
#define GPIO_HPP_INCLUDED

/// @cond
#include <stdint.h>

// #define HAL_GPIO_LIBRARY

#ifdef HAL_GPIO_LIBRARY
#else#include <stm32f0xx_ll_gpio.h>
#endif
/// @endcond
#include "../C_General/Macros.h"

namespace avp {

  struct PinBase {
    virtual void SetMode(uint32_t Mode) = 0;
    virtual uint32_t GetMode() = 0;
    virtual void SetOutputType(uint32_t Type) = 0;
    virtual uint32_t GetOutputType() = 0;


    virtual void set(bool value) = 0;
    virtual void set_high() = 0;
    virtual void set_low() = 0;
    virtual bool get() = 0;
    virtual void toggle() = 0;

    virtual ~PinBase();
  }; // PinBase

#if defined(HAL_GPIO_LIBRARY)
  enum PinMode {INPUT, OUTPUT};
  enum PinLevel {LOW, HIGH};

  template<uint32_t GPIOx, uint16_t GPIO_Pin>
  struct Pin {
    /*! sets PULLUP when mode is INPUT */
    /* CubeMX does it by itself */
    static void Config(uint32_t Mode = GPIO_MODE_INPUT, uint32_t Pull = GPIO_NOPULL,
                       uint32_t Speed = GPIO_SPEED_FREQ_LOW, uint32_t Alternate = GPIO_AF15_EVENTOUT) {
      GPIO_InitTypeDef GPIO_Init = {GPIO_Pin, Mode, Pull, Speed, Alternate};
      HAL_GPIO_Init((GPIO_TypeDef *)GPIOx,&GPIO_Init);
    } // Init
    // static constexpr char *Name() { return (const char *)PortName; }
    static void set(bool value) { HAL_GPIO_WritePin((GPIO_TypeDef *)GPIOx, GPIO_Pin, value?GPIO_PIN_SET:GPIO_PIN_RESET); }
    static void set_high() { set(true); }
    static void set_low() { set(false); }
    static bool get() { return HAL_GPIO_ReadPin((GPIO_TypeDef *)GPIOx, GPIO_Pin) != GPIO_PIN_RESET; }
    static void toggle() { HAL_GPIO_TogglePin((GPIO_TypeDef *)GPIOx, GPIO_Pin); }
  }; // Pin

#define AVP_PIN(Port,Number)  avp::Pin<uint32_t(COMB2(GPIO,Port##_BASE)),COMB2(GPIO_PIN_,Number)>

#else

  enum PinMode {INPUT=LL_GPIO_MODE_INPUT, OUTPUT=LL_GPIO_MODE_OUTPUT};
  enum PinOutputType {PUSHPULL=LL_GPIO_OUTPUT_PUSHPULL, OPENDRAIN=LL_GPIO_OUTPUT_OPENDRAIN};
  enum PinLevel {LOW, HIGH};

  template<uint32_t GPIOx, uint32_t GPIO_Pin>
  struct Pin {
    /*! sets PULLUP when mode is INPUT */
    /* CubeMX does it by itself */
    static void Config(uint32_t Mode = INPUT, uint32_t Speed = LL_GPIO_SPEED_FREQ_LOW,
                       uint32_t OutputType = PUSHPULL, uint32_t Pull = LL_GPIO_PULL_NO,
                       uint32_t Alternate = LL_GPIO_AF_0) {
      LL_GPIO_InitTypeDef GPIO_Init = {GPIO_Pin, Mode, Speed, OutputType, Pull, Alternate};
      LL_GPIO_Init((GPIO_TypeDef *)GPIOx,&GPIO_Init);
    } // Init

    // static constexpr char *Name() { return (const char *)PortName; }
    static void SetMode(uint32_t Mode) { LL_GPIO_SetPinMode((GPIO_TypeDef *)GPIOx, GPIO_Pin, Mode); }
    static uint32_t GetMode() { return LL_GPIO_GetPinMode((GPIO_TypeDef *)GPIOx, GPIO_Pin); }
    static void SetOutputType(uint32_t Type) { LL_GPIO_SetPinOutputType((GPIO_TypeDef *)GPIOx, GPIO_Pin, Type); }
    static uint32_t GetOutputType() { return LL_GPIO_GetPinOutputType((GPIO_TypeDef *)GPIOx, GPIO_Pin); }


    static void set(bool value) { if(value) set_high(); else set_low();  }
    static void set_high() { LL_GPIO_SetOutputPin((GPIO_TypeDef *)GPIOx, GPIO_Pin); }
    static void set_low() { LL_GPIO_ResetOutputPin((GPIO_TypeDef *)GPIOx, GPIO_Pin); }
    static bool get() { return LL_GPIO_IsOutputPinSet((GPIO_TypeDef *)GPIOx, GPIO_Pin); }
    static void toggle() { LL_GPIO_TogglePin((GPIO_TypeDef *)GPIOx, GPIO_Pin); }
  }; // Pin


#define AVP_PIN(Port,Number)  avp::Pin<uint32_t(COMB2(GPIO,Port)),COMB2(LL_GPIO_PIN_,Number)>

#endif

  template<uint32_t GPIOx, uint32_t GPIO_Pin>
  struct PinDyn: public PinBase {
    // static constexpr char *Name() { return (const char *)PortName; }
    virtual void SetMode(uint32_t Mode) { Pin<GPIOx,GPIO_Pin>::SetMode(Mode); }
    virtual uint32_t GetMode() { return Pin<GPIOx,GPIO_Pin>::GetMode(); }
    virtual void SetOutputType(uint32_t Type) { Pin<GPIOx,GPIO_Pin>::SetOutputType(Type); }
    virtual uint32_t GetOutputType() { return Pin<GPIOx,GPIO_Pin>::GetOutputType(); }


    virtual void set(bool value) { Pin<GPIOx,GPIO_Pin>::set(value);}
    virtual void set_high() { Pin<GPIOx,GPIO_Pin>::set_high(); }
    virtual void set_low() { Pin<GPIOx,GPIO_Pin>::set_low(); }
    virtual bool get() { return Pin<GPIOx,GPIO_Pin>::get(); }
    virtual void toggle() { Pin<GPIOx,GPIO_Pin>::toggle(); }
  }; // PinDyn

  #define AVP_PIN_DYN(Port,Number)  avp::PinDyn<uint32_t(COMB2(GPIO,Port)),COMB2(LL_GPIO_PIN_,Number)>

} // namespace avp

#endif /* GPIO_HPP_INCLUDED */

