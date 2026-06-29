/**
 * @file afe4400.h
 * @brief Driver Modul PPG Passthrough menggunakan sensor AFE4400 via SPI
 *        pada platform PULPissimo RISC-V.
 */

#ifndef AFE4400_H
#define AFE4400_H

#ifndef SOFTWARE_TEST
#include "pulp.h"
#endif

/* --- REGISTER MAP AFE4400 (sesuai Datasheet Table 6) --- */
#define AFE_CONTROL0            0x00  // Write-only: SW Reset, SPI Read mode
#define AFE_LEDCNTRL            0x22  // R/W: Arus LED1[7:0] dan LED2[7:0]
#define AFE_LED2VAL             0x2A  // Read-only: Nilai ADC 24-bit LED2 (IR)
#define AFE_ALED2VAL            0x2B  // Read-only: Nilai ADC 24-bit Ambient saat LED2 off
#define AFE_LED1VAL             0x2C  // Read-only: Nilai ADC 24-bit LED1 (Merah)
#define AFE_ALED1VAL            0x2D  // Read-only: Nilai ADC 24-bit Ambient saat LED1 off

/* --- BIT DEFINITIONS --- */
#define AFE_CONTROL0_WRITE_MODE 0x000000  // SPI_READ = 0 (mode tulis/default)
#define AFE_CONTROL0_READ_MODE  0x000001  // SPI_READ = 1 (mode baca register ADC)
#define AFE_CONTROL0_SW_RESET   0x000008  // SW_RST  = 1 (reset semua register)

/* --- FUNGSI API (INTERFACE) --- */

/**
 * @brief Membuka jalur komunikasi SPI dan melakukan SW Reset pada sensor.
 * @param spi_id Nomor jalur SPI (contoh: 0 untuk SPI_0).
 * @param cs_pin Nomor pin Chip Select.
 * @return 0 jika sukses, -1 jika gagal.
 */
int afe4400_init(int spi_id, int cs_pin);

/**
 * @brief Menulis nilai 24-bit ke register konfigurasi sensor via SPI.
 * @param reg_addr Alamat register tujuan.
 * @param data Nilai 24-bit yang akan ditulis.
 */
void afe4400_write_reg(unsigned char reg_addr, unsigned int data);

/**
 * @brief Membaca nilai 24-bit dari register ADC sensor.
 * @param reg_addr Alamat register yang akan dibaca.
 * @return Nilai raw ADC 24-bit dari sensor.
 */
unsigned int afe4400_read_reg(unsigned char reg_addr);

/**
 * @brief Mengatur arus LED ke nilai default yang aman (~6 mA) agar siap beroperasi.
 */
void afe4400_calibrate(void);

#endif // AFE4400_H