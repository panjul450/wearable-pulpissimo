#include "afe4400.h"
#include <stdio.h>

/* --- Mode Software Test (tanpa hardware, compile dengan -DSOFTWARE_TEST) --- */
#ifdef SOFTWARE_TEST

/* Simulasi nilai PPG — pola naik-turun menyerupai sinyal jantung */
static unsigned int _sim_ppg(unsigned char reg) {
    static unsigned int tick = 0;
    /* Nilai berbeda per register agar LED1, LED2, Ambient bisa dibedakan */
    unsigned int base   = (reg == AFE_LED1VAL)  ? 500000 :
                          (reg == AFE_LED2VAL)  ? 480000 : 20000;
    unsigned int amp    = (reg == AFE_ALED1VAL) ? 5000   : 100000;
    /* Tabel satu siklus gelombang sinus (10 titik) */
    unsigned int wave[] = {0, 31, 59, 81, 95, 100, 95, 81, 59, 31};
    unsigned int val    = base + (wave[tick % 10] * amp / 100);
    tick++;
    return val;
}

int afe4400_init(int spi_id, int cs_pin) {
    (void)spi_id; (void)cs_pin;
    return 0; /* Selalu sukses di mode simulasi */
}

void afe4400_write_reg(unsigned char reg_addr, unsigned int data) {
    (void)reg_addr; (void)data; /* Tidak ada aksi di mode simulasi */
}

unsigned int afe4400_read_reg(unsigned char reg_addr) {
    return _sim_ppg(reg_addr); /* Kembalikan data simulasi */
}

void afe4400_calibrate(void) {
    /* Tidak ada aksi di mode simulasi */
}

/* --- Mode Hardware (PULPissimo via SPI) --- */
#else

static spim_t *afe_spi;

/* Buffer SPI harus global/static (L2) agar bisa diakses oleh UDMA hardware. */
static unsigned char tx_buf[4];
static unsigned char rx_buf[4];

/* Inisialisasi SPI dan reset sensor */
int afe4400_init(int spi_id, int cs_pin) {
    spim_conf_t conf;
    spim_conf_init(&conf);
    conf.id           = spi_id;
    conf.cs           = cs_pin;
    conf.max_baudrate = 1000000; /* 1 MHz — aman untuk AFE4400 */

    afe_spi = spim_open(&conf);
    if (afe_spi == NULL) return -1;

    afe4400_write_reg(AFE_CONTROL0, AFE_CONTROL0_SW_RESET);   /* Reset hardware */
    afe4400_write_reg(AFE_CONTROL0, AFE_CONTROL0_WRITE_MODE); /* Kembali ke mode tulis */
    return 0;
}

/* Tulis 24-bit data ke register sensor */
void afe4400_write_reg(unsigned char reg_addr, unsigned int data) {
    /* Paket SPI: [1 byte alamat][3 byte data MSB-first] = 32 bit */
    tx_buf[0] = reg_addr;
    tx_buf[1] = (data >> 16) & 0xFF;
    tx_buf[2] = (data >>  8) & 0xFF;
    tx_buf[3] =  data        & 0xFF;
    spim_transfer(afe_spi, tx_buf, rx_buf, 32, 0);
}

/* Baca 24-bit data dari register ADC sensor */
unsigned int afe4400_read_reg(unsigned char reg_addr) {
    /* Switch ke Read Mode */
    afe4400_write_reg(AFE_CONTROL0, AFE_CONTROL0_READ_MODE);

    tx_buf[0] = reg_addr;
    tx_buf[1] = 0x00;
    tx_buf[2] = 0x00;
    tx_buf[3] = 0x00;
    
    /* Full-duplex: kirim alamat, terima 3 byte data */
    spim_transfer(afe_spi, tx_buf, rx_buf, 32, 0);

    /* Kembali ke Write Mode */
    afe4400_write_reg(AFE_CONTROL0, AFE_CONTROL0_WRITE_MODE);

    /* Gabung 3 byte menjadi nilai 24-bit */
    unsigned int result = 0;
    result |= ((unsigned int)rx_buf[1] << 16);
    result |= ((unsigned int)rx_buf[2] <<  8);
    result |=  (unsigned int)rx_buf[3];
    return result;
}

/* Kalibrasi: atur arus LED ~6mA per LED */
void afe4400_calibrate(void) {
    afe4400_write_reg(AFE_LEDCNTRL, 0x000F0F);
}

#endif /* SOFTWARE_TEST */