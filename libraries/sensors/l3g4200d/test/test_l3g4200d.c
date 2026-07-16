/*
 * Copyright (C) 2026 ICDeC
 *
 * Test Application: L3G4200D Gyroscope Sensor
 * Memakai library l3g4200d.c/.h hasil reverse-engineering dari raw
 * I2C test yang sudah terbukti berhasil.
 *
 * Usage:
 *   make all SENSOR=l3g4200d
 *   make run SENSOR=l3g4200d platform=fpga
 */

#include <stdio.h>
#include "pulp.h"
#include "l3g4200d.h"

#define CONTINUOUS_NUM_SAMPLES   300     /* 0 = infinite */
#define CONTINUOUS_DELAY_LOOPS   200000  /* delay antar pembacaan */

static void print_dps_x100(int32_t v)
{
    int sign = (v < 0) ? -1 : 1;
    int32_t av = v * sign;
    printf("%s%d.%02d", (sign < 0) ? "-" : " ", (int)(av / 100), (int)(av % 100));
}

static void continuous_gyro_read(void)
{
    printf("\n========================================\n");
    printf(" CONTINUOUS GYRO READ\n");
    if (CONTINUOUS_NUM_SAMPLES > 0)
        printf(" Jumlah sampel: %d\n", CONTINUOUS_NUM_SAMPLES);
    else
        printf(" Mode: infinite (reset board untuk stop)\n");
    printf("========================================\n\n");

    printf("  #    |  Raw X   Raw Y   Raw Z  |  dps X    dps Y    dps Z\n");
    printf("-------+------------------------+---------------------------\n");

    int sample = 0;
    while (1) {
        l3g4200d_raw_t raw;
        l3g4200d_status_t status = l3g4200d_read_raw(&raw);

        if (status != L3G4200D_OK) {
            printf("  [!] Gagal baca data (err=%d)\n", status);
            for (volatile int d = 0; d < CONTINUOUS_DELAY_LOOPS; d++);
            continue;
        }

        l3g4200d_dps_x100_t dps;
        l3g4200d_read_dps_x100(&dps); /* pakai raw yang sama secara implisit lewat read ulang */

        printf(" %4d  | %6d  %6d  %6d  | ", sample, raw.x, raw.y, raw.z);
        print_dps_x100(dps.x_x100); printf("   ");
        print_dps_x100(dps.y_x100); printf("   ");
        print_dps_x100(dps.z_x100); printf(" dps\n");

        sample++;
        if (CONTINUOUS_NUM_SAMPLES > 0 && sample >= CONTINUOUS_NUM_SAMPLES)
            break;

        for (volatile int d = 0; d < CONTINUOUS_DELAY_LOOPS; d++);
    }

    printf("\n  Selesai: %d sampel terbaca.\n", sample);
}

int main()
{
    l3g4200d_config_t cfg;
    l3g4200d_status_t status;
    int pass_count = 0;
    int fail_count = 0;

    printf("========================================\n");
    printf(" L3G4200D Gyroscope Test (via library)\n");
    printf(" ICDeC PULPissimo FPGA Board\n");
    printf("========================================\n\n");

    /* ---- Test 1: Default config ---- */
    printf("[TEST 1] Loading default configuration...\n");
    status = l3g4200d_default_config(&cfg);
    if (status == L3G4200D_OK) {
        printf("  PASS: addr=0x%02X, freq=%u, range=%d\n",
               cfg.i2c_addr, cfg.i2c_freq, cfg.range);
        pass_count++;
    } else {
        printf("  FAIL (err=%d)\n", status);
        fail_count++;
    }
    printf("\n");

    /* ---- Test 2: Init (buka I2C, verifikasi WHO_AM_I, konfigurasi) ---- */
    printf("[TEST 2] Initializing L3G4200D...\n");
    status = l3g4200d_init(&cfg);
    if (status == L3G4200D_OK) {
        printf("  PASS: Sensor initialized on addr=0x%02X\n", cfg.i2c_addr);
        pass_count++;
    } else {
        printf("  FAIL (err=%d)\n", status);
        fail_count++;
        printf("\n========================================\n");
        printf(" RESULTS: %d PASSED, %d FAILED\n", pass_count, fail_count);
        printf("========================================\n");
        return -1;
    }
    printf("\n");

    /* ---- Test 3: WHO_AM_I manual ---- */
    printf("[TEST 3] Reading WHO_AM_I register...\n");
    uint8_t who_am_i = 0;
    status = l3g4200d_who_am_i(&who_am_i);
    if (status == L3G4200D_OK && who_am_i == L3G4200D_WHO_AM_I_VALUE) {
        printf("  PASS: WHO_AM_I = 0x%02X\n", who_am_i);
        pass_count++;
    } else {
        printf("  FAIL: WHO_AM_I = 0x%02X (err=%d)\n", who_am_i, status);
        fail_count++;
    }
    printf("\n");

    /* ---- Test 4: Baca raw + dps sekali ---- */
    printf("[TEST 4] Membaca data gyro sekali...\n");
    l3g4200d_raw_t raw;
    l3g4200d_dps_x100_t dps;
    status = l3g4200d_read_raw(&raw);
    if (status == L3G4200D_OK) {
        l3g4200d_read_dps_x100(&dps);
        printf("  Raw: X=%d Y=%d Z=%d\n", raw.x, raw.y, raw.z);
        printf("  dps: X="); print_dps_x100(dps.x_x100);
        printf(" Y="); print_dps_x100(dps.y_x100);
        printf(" Z="); print_dps_x100(dps.z_x100);
        printf("\n  PASS\n");
        pass_count++;
    } else {
        printf("  FAIL (err=%d)\n", status);
        fail_count++;
    }
    printf("\n");

    /* ---- Results ---- */
    printf("========================================\n");
    printf(" RESULTS: %d PASSED, %d FAILED\n", pass_count, fail_count);
    printf("========================================\n");

    if (fail_count == 0) {
        continuous_gyro_read();
    }

    l3g4200d_deinit();

    return (fail_count == 0) ? 0 : -1;
}

void pe_start(void)
{
}