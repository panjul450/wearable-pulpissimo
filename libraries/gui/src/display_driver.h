/**
 * @file    display_driver.h
 * @brief   NusaCore display driver — platform-agnostic API
 *
 * On Linux  (PLATFORM_LINUX):  SDL2 window via LVGL's built-in SDL driver
 * On RISC-V (bare-metal):      I2C OLED via the ICDeC OLED module
 *
 * The rest of the application only calls display_driver_init() and never
 * touches platform-specific code directly.
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "libs/lvgl/lvgl.h"

/* Physical display size — must match the OLED hardware */
#define DISPLAY_HOR_RES  128
#define DISPLAY_VER_RES   64

/**
 * @brief  Initialise the display and register it with LVGL.
 *
 * @return Pointer to the LVGL display object.
 *         Returns NULL on failure (I2C init error, SDL init error, etc.)
 *
 * Call this once, after lv_init(), before creating any LVGL widgets.
 */
lv_display_t *display_driver_init(void);

#endif /* DISPLAY_DRIVER_H */
