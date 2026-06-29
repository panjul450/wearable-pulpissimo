/**
 * @file    input_driver.c
 * @brief   NusaCore input driver — implementation
 *
 * Key design: gesture_feed() + gesture_poll() are called INSIDE the
 * LVGL indev read_cb.  This is simpler than a separate timer and avoids
 * any LVGL v9 API issues with lv_indev_read().
 *
 * ┌──────────────────────────────────────────────────────────┐
 * │  LVGL calls read_cb every LV_INDEV_DEF_READ_PERIOD ms   │
 * │                                                          │
 * │  read_cb (platform-specific)                             │
 * │    │                                                     │
 * │    ├─ get pressed/x/y  (SDL mouse  OR  I2C touch)       │
 * │    ├─ gesture_feed(pressed, x, y)                        │
 * │    ├─ gesture_poll()  → fire g_gesture_cb if event ready │
 * │    └─ fill lv_indev_data_t  → return to LVGL            │
 * └──────────────────────────────────────────────────────────┘
 */

#include "input_driver.h"
#include "display_driver.h"   /* DISPLAY_HOR_RES, DISPLAY_VER_RES */
#include "gesture.h"
#include <stdio.h>

/* =========================================================================
 * Module state
 * ========================================================================= */
static gesture_cb_t g_gesture_cb = NULL;
static lv_indev_t  *g_indev      = NULL;

/* =========================================================================
 * Common: feed gesture engine and report to LVGL
 * Called at the end of every platform-specific read_cb.
 * ========================================================================= */
static void feed_and_report(bool pressed, int16_t x, int16_t y,
                             lv_indev_data_t *data)
{
    gesture_feed(pressed, x, y);

    gesture_event_t ev;
    if (gesture_poll(&ev) && g_gesture_cb != NULL) {
        printf("[INPUT] Gesture: %-14s  (%3d,%2d)→(%3d,%2d)  %lums\n",
               gesture_type_str(ev.type),
               ev.start_x, ev.start_y, ev.end_x, ev.end_y,
               (unsigned long)ev.duration_ms);
        g_gesture_cb(&ev);
    }

    data->point.x = x;
    data->point.y = y;
    data->state   = pressed ? LV_INDEV_STATE_PRESSED
                            : LV_INDEV_STATE_RELEASED;
}

/* =========================================================================
 * ── LINUX / SDL2 PATH ────────────────────────────────────────────────────
 *
 * We implement our own read_cb that polls SDL_GetMouseState() directly.
 * This avoids using lv_sdl_mouse_create() and removes any dependency on
 * LVGL v9's lv_indev_read() API.
 *
 * SDL_GetMouseState() returns the *current* hardware state (not from the
 * event queue), so it never conflicts with the SDL display driver's own
 * event processing.
 * ========================================================================= */
#ifdef PLATFORM_LINUX

#include <SDL2/SDL.h>

/* Tick source for gesture engine (Linux) */
uint32_t gesture_tick_ms(void)
{
    return (uint32_t)SDL_GetTicks();  /* ms since SDL_Init — no extra deps */
}

static void sdl_mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;

    int mx, my;
    uint32_t buttons = SDL_GetMouseState(&mx, &my);
    bool pressed = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

    /*
     * SDL window is (DISPLAY_HOR_RES × LV_SDL_ZOOM_FACTOR) pixels wide.
     * SDL_GetMouseState() returns window-space coords.
     * Divide by zoom factor to get LVGL logical coords (0…DISPLAY_HOR_RES-1).
     */
    int16_t x = (int16_t)(mx / LV_SDL_ZOOM_FACTOR);
    int16_t y = (int16_t)(my / LV_SDL_ZOOM_FACTOR);

    /* Clamp to display bounds */
    if (x < 0)                   x = 0;
    if (x >= DISPLAY_HOR_RES)    x = DISPLAY_HOR_RES - 1;
    if (y < 0)                   y = 0;
    if (y >= DISPLAY_VER_RES)    y = DISPLAY_VER_RES - 1;

    feed_and_report(pressed, x, y, data);
}

void input_driver_init(lv_display_t *display)
{
    printf("[INPUT] Platform: Linux / SDL2 mouse\n");
    printf("[INPUT] Left-click = tap | Drag = swipe | "
           "Double-click = double tap | Hold = long press\n");

    gesture_init();

    g_indev = lv_indev_create();
    lv_indev_set_type(g_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(g_indev, sdl_mouse_read_cb);
    lv_indev_set_display(g_indev, display);

    printf("[INPUT] SDL2 mouse indev registered\n");
}

/* =========================================================================
 * ── RISC-V / TOUCH MODULE PATH ───────────────────────────────────────────
 *
 * Replace the two TODO sections when the ICDeC touch module is ready.
 * See docs/extending.md for step-by-step instructions.
 * ========================================================================= */
#else /* RISC-V bare-metal */

/* TODO: include the ICDeC touch module header, e.g.:
 * #include "touch.h"
 */

/* Tick source for gesture engine (RISC-V) */
extern volatile uint32_t g_tick_ms;   /* declared in main.c */
uint32_t gesture_tick_ms(void) { return g_tick_ms; }

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;

    bool    pressed = false;
    int16_t x       = 0;
    int16_t y       = 0;

    /*
     * TODO: Replace stub with ICDeC touch module call, e.g.:
     *
     *   touch_point_t pt;
     *   pressed = touch_read(&pt);
     *   x = pt.x;
     *   y = pt.y;
     */

    feed_and_report(pressed, x, y, data);
}

void input_driver_init(lv_display_t *display)
{
    printf("[INPUT] Platform: RISC-V bare-metal (touch module)\n");

    /*
     * TODO: Initialise touch hardware, e.g.:
     *   if (touch_init() != 0) {
     *       printf("[INPUT] ERROR: touch init failed\n");
     *       return;
     *   }
     */
    printf("[INPUT] Touch init stub — replace with touch_init()\n");

    gesture_init();

    g_indev = lv_indev_create();
    lv_indev_set_type(g_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(g_indev, touch_read_cb);
    lv_indev_set_display(g_indev, display);

    printf("[INPUT] Touch indev registered\n");
}

#endif /* PLATFORM_LINUX */

/* =========================================================================
 * Platform-independent
 * ========================================================================= */

void input_driver_set_gesture_cb(gesture_cb_t cb)
{
    g_gesture_cb = cb;
}
