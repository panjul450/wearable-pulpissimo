/**
 * @file    display_driver.c
 * @brief   NusaCore display driver — implementation
 *
 * Two code paths selected at compile time:
 *
 *   PLATFORM_LINUX  → SDL2 window (LVGL built-in SDL driver)
 *                     Displays a 4× zoomed 128×64 window on desktop.
 *                     No root or special hardware access needed.
 *
 *   (default)       → I2C OLED via the ICDeC OLED module.
 *                     When OLED module is ready, replace the TODO stubs.
 *
 * Both paths produce a valid lv_display_t* so the rest of the app is
 * completely unaware of which backend is in use.
 */

#include "display_driver.h"
#include <stdio.h>
#include <string.h>

/* =========================================================================
 * ── LINUX / SDL2 PATH ────────────────────────────────────────────────────
 * ========================================================================= */
#ifdef PLATFORM_LINUX

#include "libs/lvgl/src/drivers/sdl/lv_sdl_window.h"

lv_display_t *display_driver_init(void)
{
    printf("[DISP] Platform: Linux/SDL2\n");
    printf("[DISP] Logical resolution: %d × %d (OLED size)\n",
           DISPLAY_HOR_RES, DISPLAY_VER_RES);
    printf("[DISP] SDL window: %d × %d (4× zoom)\n",
           DISPLAY_HOR_RES * LV_SDL_ZOOM_FACTOR,
           DISPLAY_VER_RES * LV_SDL_ZOOM_FACTOR);

    /*
     * lv_sdl_window_create() opens an SDL2 window, registers a flush_cb
     * that blits LVGL's render buffer to an SDL texture, and handles
     * window resize / quit events internally.
     *
     * SDL_HOR_RES / SDL_VER_RES are defined in lv_conf.h and must match
     * DISPLAY_HOR_RES / DISPLAY_VER_RES.
     */
    lv_display_t *disp = lv_sdl_window_create(DISPLAY_HOR_RES, DISPLAY_VER_RES);
    if (disp == NULL) {
        fprintf(stderr, "[DISP] ERROR: lv_sdl_window_create() failed\n");
        return NULL;
    }

    printf("[DISP] SDL2 display ready\n");
    return disp;
}

/* =========================================================================
 * ── RISC-V / OLED PATH ───────────────────────────────────────────────────
 *
 * Replace the TODO sections with the ICDeC OLED module API once it is
 * available.  The interface contract is:
 *
 *   oled_init()                      — power-on and configure the OLED
 *   oled_flush(x1,y1,x2,y2,buf)     — write a pixel region to the display
 *
 * The OLED is 128×64 monochrome.  LVGL renders at 16-bit colour (see
 * lv_conf.h); the flush_cb converts to 1-bit here before sending to OLED.
 * ========================================================================= */
#else /* RISC-V bare-metal */

/* TODO: Include the ICDeC OLED module header when available
 * #include "oled.h"
 */

/* Internal draw buffer — partial (10 rows) to keep RAM usage low */
#define DISP_BUF_ROWS   10
#define DISP_BUF_BYTES  (DISPLAY_HOR_RES * DISP_BUF_ROWS * (LV_COLOR_DEPTH / 8))

static uint8_t g_draw_buf[DISP_BUF_BYTES];

/**
 * @brief  Convert LVGL colour buffer → 1-bit and push to OLED.
 *         Called by LVGL whenever a region of the screen is dirty.
 */
static void oled_flush_cb(lv_display_t *disp,
                           const lv_area_t *area,
                           uint8_t *px_map)
{
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    /*
     * px_map points to LVGL's draw buffer.
     * At LV_COLOR_DEPTH=16, each pixel is 2 bytes (RGB565).
     * We send the raw buffer to the OLED module and let it handle
     * 1-bit conversion — OR convert here if the OLED API expects 1-bit.
     *
     * TODO: Replace with actual OLED module call, e.g.:
     *   oled_flush(area->x1, area->y1, area->x2, area->y2, px_map);
     */
    (void)w; (void)h; (void)px_map;
    printf("[DISP] Flush: (%d,%d)→(%d,%d) [stub — replace with oled_flush()]\n",
           (int)area->x1, (int)area->y1, (int)area->x2, (int)area->y2);

    /* IMPORTANT: always call this to unblock LVGL's render pipeline */
    lv_display_flush_ready(disp);
}

lv_display_t *display_driver_init(void)
{
    printf("[DISP] Platform: RISC-V bare-metal (OLED)\n");

    /* TODO: Initialise OLED hardware
     * if (oled_init() != 0) {
     *     printf("[DISP] ERROR: OLED init failed\n");
     *     return NULL;
     * }
     */
    printf("[DISP] OLED init stub (replace with oled_init())\n");

    /* Create LVGL display descriptor */
    lv_display_t *disp = lv_display_create(DISPLAY_HOR_RES, DISPLAY_VER_RES);
    if (disp == NULL) return NULL;

    /* Register partial draw buffer and flush callback */
    lv_display_set_flush_cb(disp, oled_flush_cb);
    lv_display_set_buffers(disp,
                            g_draw_buf, NULL,
                            sizeof(g_draw_buf),
                            LV_DISPLAY_RENDER_MODE_PARTIAL);

    printf("[DISP] OLED display registered: %d × %d\n",
           DISPLAY_HOR_RES, DISPLAY_VER_RES);
    return disp;
}

#endif /* PLATFORM_LINUX */
