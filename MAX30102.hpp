/*
 * MAX30102.hpp
 *
 *  Created on: Apr 21, 2022
 *      Author: panasyuk 
 */

#ifndef INCLUDE_AVP_LIBS_ARM_HPP_
#define INCLUDE_AVP_LIBS_ARM_HPP_

#include <stdint.h>
#include <string.h>
#include <stm32f3xx_hal_i2c.h>

class MAX30102 {
static constexpr uint16_t I2C_ADDR = 0x57;
static constexpr uint32_t I2C_TIMEOUT = 1000;

static constexpr uint8_t BYTES_PER_SAMPLE = 6;
static constexpr uint8_t SAMPLE_LEN_MAX = 32;

static constexpr uint8_t INTERRUPT_STATUS_1 = 0x00;
static constexpr uint8_t INTERRUPT_STATUS_2 = 0x01;
static constexpr uint8_t INTERRUPT_ENABLE_1 = 0x02;
static constexpr uint8_t INTERRUPT_ENABLE_2 = 0x03;
static constexpr uint8_t INTERRUPT_A_FULL = 7;
static constexpr uint8_t INTERRUPT_PPG_RDY = 6;
static constexpr uint8_t INTERRUPT_ALC_OVF = 5;
static constexpr uint8_t INTERRUPT_DIE_TEMP_RDY = 1;

static constexpr uint8_t FIFO_WR_PTR = 0x04;
static constexpr uint8_t OVF_COUNTER = 0x05;
static constexpr uint8_t FIFO_RD_PTR = 0x06;

static constexpr uint8_t FIFO_DATA = 0x07;

static constexpr uint8_t FIFO_CONFIG = 0x08;
static constexpr uint8_t FIFO_CONFIG_SMP_AVE = 5;
static constexpr uint8_t FIFO_CONFIG_ROLL_OVER_EN = 4;
static constexpr uint8_t FIFO_CONFIG_FIFO_A_FULL = 0;

static constexpr uint8_t MODE_CONFIG = 0x09;
static constexpr uint8_t MODE_SHDN = 7;
static constexpr uint8_t MODE_RESET = 6;
static constexpr uint8_t MODE_MODE = 0;

static constexpr uint8_t SPO2_CONFIG = 0x0a;
static constexpr uint8_t SPO2_ADC_RGE = 5;
static constexpr uint8_t SPO2_SR = 2;
static constexpr uint8_t SPO2_LEW_PW = 0;

static constexpr uint8_t LED_IR_PA1 = 0x0c;
static constexpr uint8_t LED_RED_PA2 = 0x0d;

static constexpr uint8_t MULTI_LED_CTRL_1 = 0x11;
static constexpr uint8_t MULTI_LED_CTRL_SLOT2 = 4;
static constexpr uint8_t MULTI_LED_CTRL_SLOT1 = 0;
static constexpr uint8_t MULTI_LED_CTRL_2 = 0x12;
static constexpr uint8_t MULTI_LED_CTRL_SLOT4 = 4;
static constexpr uint8_t MULTI_LED_CTRL_SLOT3 = 0;

static constexpr uint8_t DIE_TINT = 0x1f;
static constexpr uint8_t DIE_TFRAC = 0x20;
static constexpr float DIE_TFRAC_INCREMENT = 0.0625f;
static constexpr uint8_t DIE_TEMP_CONFIG = 0x21;
static constexpr uint8_t DIE_TEMP_EN = 1;

typedef enum mode_t { heart_rate = 0x02, spo2 = 0x03, multi_led = 0x07} mode_t;

typedef enum smp_ave_t {smp_ave_1, smp_ave_2, smp_ave_4, smp_ave_8, smp_ave_16, smp_ave_32} smp_ave_t;

typedef enum sr_t {sr_50, sr_100, sr_200, sr_400, sr_800, sr_1000, sr_1600, sr_3200} sr_t;

typedef enum led_pw_t {pw_15_bit, pw_16_bit, pw_17_bit, pw_18_bit} led_pw_t;

typedef enum adc_t {adc_2048, adc_4096, adc_8192, adc_16384} adc_t;

typedef enum multi_led_ctrl_t {led_off, led_red, led_ir} multi_led_ctrl_t;

I2C_HandleTypeDef const *pi2c;
uint32_t ir_samples[SAMPLE_LEN_MAX];
uint32_t red_samples[SAMPLE_LEN_MAX];
uint8_t interrupt_flag;

public:
  MAX30102(I2C_HandleTypeDef &hi2c): pi2c(&hi2c), interrupt_flag(0) {
    for(auto &x: ir_samples) x = 0;
    for(auto &x: red_samples) x = 0;
  } // MAX30102

  HAL_StatusTypeDef IsReady() {
    return  HAL_I2C_IsDeviceReady(pi2c, I2C_ADDR << 1, 10, I2C_TIMEOUT);
  }
  /**
   * @brief Write buffer of buflen bytes to a register of the MAX30102.
   *
   * @param reg Register address to write to.
   * @param buf Pointer containing the bytes to write.
   * @param buflen Number of bytes to write.
   */
  HAL_StatusTypeDef write(uint8_t reg, uint8_t *buf, uint16_t buflen) {
    uint8_t payload[buflen + 1] = { reg }; // I have to prepend register number
    if (buf != NULL && buflen != 0) memcpy((void *)(payload + 1), (void *)buf, buflen);
    return HAL_I2C_Master_Transmit(pi2c, I2C_ADDR << 1, payload, buflen + 1, I2C_TIMEOUT);
  } // write

  /**
   * @brief Read buflen bytes from a register of the MAX30102 and store to buffer.
   *
   * @param reg Register address to read from.
   * @param buf Pointer to the array to write to.
   * @param buflen Number of bytes to read.
   */
  HAL_StatusTypeDef read(uint8_t reg, uint8_t *buf, uint16_t buflen) {
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(pi2c, I2C_ADDR << 1, &reg, 1, I2C_TIMEOUT);
    if(status == HAL_OK) return HAL_I2C_Master_Receive(pi2c, I2C_ADDR << 1, buf, buflen, I2C_TIMEOUT);
    else return status;
  }

  /**
   * @brief Reset the sensor.
   */
  HAL_StatusTypeDef reset() {
      uint8_t val = 0x40;
      return write(MODE_CONFIG, &val, 1);
  } // reset

   /**
   * @brief control an interrupt.
   *
   * @param enable bool - true to enable, false to disable.
   */
  HAL_StatusTypeDef able_interrupt(bool enable, uint8_t interrupt) {
    uint8_t reg = 0;
    HAL_StatusTypeDef status = read(INTERRUPT_ENABLE_1, &reg, 1);
    if(status == HAL_OK) {
      if(enable) reg |= 0x01 << interrupt;
      else reg &= ~(0x01 << interrupt);
      return write(INTERRUPT_ENABLE_1, &reg, 1);
    } else return status;
  } // able_interrupt

  HAL_StatusTypeDef able_a_full(bool enable) {
    return able_interrupt(enable, INTERRUPT_A_FULL);
  } // able_a_full

  HAL_StatusTypeDef able_ppg_rdy(bool  enable) {
    return able_interrupt(enable, INTERRUPT_PPG_RDY);
  }

  HAL_StatusTypeDef able_alc_ovf(bool  enable) {
    return able_interrupt(enable, INTERRUPT_ALC_OVF);
  }

  HAL_StatusTypeDef able_die_temp_rdy(bool enable) {
      uint8_t reg = enable?0x01 << INTERRUPT_DIE_TEMP_RDY:0;
      return write(INTERRUPT_ENABLE_2, &reg, 1);
  }

  /**
   * @brief Enable temperature measurement.
   *
   * @param enable bool - true to enable, false to disable.
   */
  HAL_StatusTypeDef  set_die_temp_en(bool enable) {
      uint8_t reg = enable?0x01 << DIE_TEMP_EN:0;
      return write(DIE_TEMP_CONFIG, &reg, 1);
  }

  /**
   * @brief Read FIFO content and store to buffer
   */
  void read_fifo() {
      // First transaction: Get the FIFO_WR_PTR
      uint8_t wr_ptr = 0, rd_ptr = 0;
      read(FIFO_WR_PTR, &wr_ptr, 1);
      read(FIFO_RD_PTR, &rd_ptr, 1);

      int8_t num_samples;

      num_samples = (int8_t)wr_ptr - (int8_t)rd_ptr;
      if (num_samples < 1) {
          num_samples += 32;
      }

      // Second transaction: Read NUM_SAMPLES_TO_READ samples from the FIFO
      for (int8_t i = 0; i < num_samples; i++) {
          uint8_t sample[6];
          read(FIFO_DATA, sample, 6);
          uint32_t ir_sample = ((uint32_t)(sample[0] << 16) | (uint32_t)(sample[1] << 8) |
              (uint32_t)(sample[3])) & 0x3ffff;
          uint32_t red_sample = ((uint32_t)(sample[3] << 16) | (uint32_t)(sample[4] << 8) |
              (uint32_t)(sample[5])) & 0x3ffff;
          ir_samples[i] = ir_sample;
          red_samples[i] = red_sample;
          plot(ir_sample, red_sample);
      }
  } // read_fifo

 /**
   * @brief Read interrupt status registers (0x00 and 0x01) and perform corresponding tasks.
   */
  void interrupt_handler() {
    uint8_t reg[2] = {0x00};
    // Interrupt flag in registers 0x00 and 0x01 are cleared on read
    HAL_StatusTypeDef status = read(INTERRUPT_STATUS_1, reg, 2);

    if(status == HAL_OK) {
      if ((reg[0] >> INTERRUPT_A_FULL) & 0x01)
      {
        // FIFO almost full
        read_fifo(obj);
      }

      if ((reg[0] >> INTERRUPT_PPG_RDY) & 0x01) {
        // New FIFO data ready
      }

      if ((reg[0] >> INTERRUPT_ALC_OVF) & 0x01)
      {
        // Ambient light overflow
      }

      if ((reg[1] >> INTERRUPT_DIE_TEMP_RDY) & 0x01)
      {
        // Temperature data ready
        int8_t temp_int;
        uint8_t temp_frac;
        read_temp(&temp_int, &temp_frac);
        // float temp = temp_int + 0.0625f * temp_frac;
      }

    }
    // Reset interrupt flag
    _interrupt_flag = 0;
  }

  /**
   * @brief Shutdown the sensor.
   *
   * @param obj Pointer to t object instance.
   * @param shdn Shutdown bit.
   */
  void shutdown(uint8_t shdn)
  {
      uint8_t config;
      read(MODE_CONFIG, &config, 1);
      config = (config & 0x7f) | (shdn << MODE_SHDN);
      write(MODE_CONFIG, &config, 1);
  }

  /**
   * @brief Set measurement mode.
   *
   * @param obj Pointer to t object instance.
   * @param mode Measurement mode enum (mode_t).
   */
  void set_mode(mode_t mode)
  {
      uint8_t config;
      read(MODE_CONFIG, &config, 1);
      config = (config & 0xf8) | mode;
      write(MODE_CONFIG, &config, 1);
      clear_fifo(obj);
  }

  /**
   * @brief Set sampling rate.
   *
   * @param obj Pointer to t object instance.
   * @param sr Sampling rate enum (spo2_st_t).
   */
  void set_sampling_rate(sr_t sr)
  {
      uint8_t config;
      read(SPO2_CONFIG, &config, 1);
      config = (config & 0x63) << SPO2_SR;
      write(SPO2_CONFIG, &config, 1);
  }

  /**
   * @brief Set led pulse width.
   *
   * @param obj Pointer to t object instance.
   * @param pw Pulse width enum (led_pw_t).
   */
  void set_led_pulse_width(led_pw_t pw)
  {
      uint8_t config;
      read(SPO2_CONFIG, &config, 1);
      config = (config & 0x7c) | (pw << SPO2_LEW_PW);
      write(SPO2_CONFIG, &config, 1);
  }

  /**
   * @brief Set ADC resolution.
   *
   * @param obj Pointer to t object instance.
   * @param adc ADC resolution enum (adc_t).
   */
  void set_adc_resolution(adc_t adc)
  {
      uint8_t config;
      read(SPO2_CONFIG, &config, 1);
      config = (config & 0x1f) | (adc << SPO2_ADC_RGE);
      write(SPO2_CONFIG, &config, 1);
  }

  /**
   * @brief Set LED current.
   *
   * @param obj Pointer to t object instance.
   * @param ma LED current float (0 < ma < 51.0).
   */
  void set_led_current_1(float ma)
  {
      uint8_t pa = ma / 0.2;
      write(LED_IR_PA1, &pa, 1);
  }

  /**
   * @brief Set LED current.
   *
   * @param obj Pointer to t object instance.
   * @param ma LED current float (0 < ma < 51.0).
   */
  void set_led_current_2(float ma)
  {
      uint8_t pa = ma / 0.2;
      write(LED_RED_PA2, &pa, 1);
  }

  /**
   * @brief Set slot mode when in multi-LED mode.
   *
   * @param obj Pointer to t object instance.
   * @param slot1 Slot 1 mode enum (multi_led_ctrl_t).
   * @param slot2 Slot 2 mode enum (multi_led_ctrl_t).
   */
  void set_multi_led_slot_1_2(multi_led_ctrl_t slot1, multi_led_ctrl_t slot2)
  {
      uint8_t val = 0;
      val |= ((slot1 << MULTI_LED_CTRL_SLOT1) | (slot2 << MULTI_LED_CTRL_SLOT2));
      write(MULTI_LED_CTRL_1, &val, 1);
  }

  /**
   * @brief Set slot mode when in multi-LED mode.
   *
   * @param obj Pointer to t object instance.
   * @param slot1 Slot 1 mode enum (multi_led_ctrl_t).
   * @param slot2 Slot 2 mode enum (multi_led_ctrl_t).
   */
  void set_multi_led_slot_3_4(multi_led_ctrl_t slot3, multi_led_ctrl_t slot4)
  {
      uint8_t val = 0;
      val |= ((slot3 << MULTI_LED_CTRL_SLOT3) | (slot4 << MULTI_LED_CTRL_SLOT4));
      write(MULTI_LED_CTRL_2, &val, 1);
  }

  /**
   * @brief
   *
   * @param obj Pointer to t object instance.
   * @param smp_ave
   * @param roll_over_en Roll over enabled(1) or disabled(0).
   * @param fifo_a_full Number of empty samples when A_FULL interrupt issued (0 < fifo_a_full < 15).
   */
  void set_fifo_config(smp_ave_t smp_ave, uint8_t roll_over_en, uint8_t fifo_a_full)
  {
      uint8_t config = 0x00;
      config |= smp_ave << FIFO_CONFIG_SMP_AVE;
      config |= ((roll_over_en & 0x01) << FIFO_CONFIG_ROLL_OVER_EN);
      config |= ((fifo_a_full & 0x0f) << FIFO_CONFIG_FIFO_A_FULL);
      write(FIFO_CONFIG, &config, 1);
  }

  /**
   * @brief Clear all FIFO pointers in the sensor.
   *
   * @param obj Pointer to t object instance.
   */
  void clear_fifo()
  {
      uint8_t val = 0x00;
      write(FIFO_WR_PTR, &val, 3);
      write(FIFO_RD_PTR, &val, 3);
      write(OVF_COUNTER, &val, 3);
  }

  /**
   * @brief Read die temperature.
   *
   * @param obj Pointer to t object instance.
   * @param temp_int Pointer to store the integer part of temperature. Stored in 2's complement format.
   * @param temp_frac Pointer to store the fractional part of temperature. Increments of 0.0625 deg C.
   */

  void read_temp(int8_t *temp_int, uint8_t *temp_frac)
  {
      read(DIE_TINT, (uint8_t *)temp_int, 1);
      read(DIE_TFRAC, temp_frac, 1);
  }


}; // class MAX30102







#endif /* INCLUDE_AVP_LIBS_ARM_HPP_ */
