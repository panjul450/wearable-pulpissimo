/**
 * @file max30102_regs.h
 * @brief MAX30102 Register Map Definitions
 *
 * Register addresses, bit masks, and configuration constants for the
 * MAX30102 Pulse Oximeter and Heart Rate Sensor.
 *
 * Reference: MAX30102 Datasheet
 */

#ifndef MAX30102_REGS_H
#define MAX30102_REGS_H

// -------------------------------------------------------------------------
// I2C ADDRESS
// -------------------------------------------------------------------------
// Default 7-bit I2C address for MAX30102
#define MAX30102_I2C_ADDR             0x57

// -------------------------------------------------------------------------
// REGISTER MAP
// -------------------------------------------------------------------------

// Status Registers
#define MAX30102_REG_INT_STAT_1       0x00
#define MAX30102_REG_INT_STAT_2       0x01
#define MAX30102_REG_INT_ENABLE_1     0x02
#define MAX30102_REG_INT_ENABLE_2     0x03

// FIFO Registers
#define MAX30102_REG_FIFO_WR_PTR      0x04
#define MAX30102_REG_OVF_COUNTER      0x05
#define MAX30102_REG_FIFO_RD_PTR      0x06
#define MAX30102_REG_FIFO_DATA        0x07

// Configuration Registers
#define MAX30102_REG_FIFO_CONFIG      0x08
#define MAX30102_REG_MODE_CONFIG      0x09
#define MAX30102_REG_SPO2_CONFIG      0x0A
#define MAX30102_REG_LED1_PA          0x0C // Red LED
#define MAX30102_REG_LED2_PA          0x0D // IR LED
#define MAX30102_REG_MULTI_LED_CTRL1  0x11
#define MAX30102_REG_MULTI_LED_CTRL2  0x12

// Temperature Registers
#define MAX30102_REG_TEMP_INT         0x1F
#define MAX30102_REG_TEMP_FRAC        0x20
#define MAX30102_REG_TEMP_CONFIG      0x21

// Part ID Registers
#define MAX30102_REG_REV_ID           0xFE
#define MAX30102_REG_PART_ID          0xFF

// Expected Part ID value for MAX30102
#define MAX30102_EXPECTED_PART_ID     0x15

// -------------------------------------------------------------------------
// BIT MASKS AND CONFIGURATION VALUES
// -------------------------------------------------------------------------

// INT_ENABLE_1
#define MAX30102_INT_A_FULL_EN        (1 << 7)
#define MAX30102_INT_PPG_RDY_EN       (1 << 6)
#define MAX30102_INT_ALC_OVF_EN       (1 << 5)

// INT_ENABLE_2
#define MAX30102_INT_DIE_TEMP_RDY_EN  (1 << 1)

// MODE_CONFIG
#define MAX30102_MODE_SHDN            (1 << 7)
#define MAX30102_MODE_RESET           (1 << 6)
#define MAX30102_MODE_HR              0x02 // Heart Rate only (Red only)
#define MAX30102_MODE_SPO2            0x03 // SpO2 (Red and IR)
#define MAX30102_MODE_MULTI_LED       0x07 // Multi-LED mode

// SPO2_CONFIG
// ADC Range (SpO2 ADC Range Control)
#define MAX30102_SPO2_ADC_2048        (0x00 << 5)
#define MAX30102_SPO2_ADC_4096        (0x01 << 5)
#define MAX30102_SPO2_ADC_8192        (0x02 << 5)
#define MAX30102_SPO2_ADC_16384       (0x03 << 5)

// Sample Rate (SpO2 Sample Rate Control)
#define MAX30102_SPO2_SR_50           (0x00 << 2)
#define MAX30102_SPO2_SR_100          (0x01 << 2)
#define MAX30102_SPO2_SR_200          (0x02 << 2)
#define MAX30102_SPO2_SR_400          (0x03 << 2)
#define MAX30102_SPO2_SR_800          (0x04 << 2)
#define MAX30102_SPO2_SR_1000         (0x05 << 2)
#define MAX30102_SPO2_SR_1600         (0x06 << 2)
#define MAX30102_SPO2_SR_3200         (0x07 << 2)

// LED Pulse Width Control
#define MAX30102_SPO2_PW_69           0x00 // 15-bit resolution
#define MAX30102_SPO2_PW_118          0x01 // 16-bit resolution
#define MAX30102_SPO2_PW_215          0x02 // 17-bit resolution
#define MAX30102_SPO2_PW_411          0x03 // 18-bit resolution

// FIFO_CONFIG
#define MAX30102_SMP_AVE_1            (0x00 << 5)
#define MAX30102_SMP_AVE_2            (0x01 << 5)
#define MAX30102_SMP_AVE_4            (0x02 << 5)
#define MAX30102_SMP_AVE_8            (0x03 << 5)
#define MAX30102_SMP_AVE_16           (0x04 << 5)
#define MAX30102_SMP_AVE_32           (0x05 << 5)
#define MAX30102_FIFO_ROLLOVER_EN     (1 << 4)

#endif // MAX30102_REGS_H