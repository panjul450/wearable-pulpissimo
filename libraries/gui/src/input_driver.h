/**
 * @file    input_driver.h
 * @brief   NusaCore input driver — platform-agnostic API
 *
 * Responsibilities:
 *   1. Read raw touch coordinates from hardware (or SDL mouse on Linux)
 *   2. Feed them into the gesture engine (gesture.h)
 *   3. Register the result as an LVGL indev so widgets respond normally
 *   4. Fire the gesture callback when a gesture is detected
 *
 * The caller registers a gesture_cb_t to receive CLICK / DOUBLE_CLICK /
 * SWIPE_* / LONG_PRESS events.  This is how the application reacts to
 * user input (e.g. navigating between screens on swipe).
 */

#ifndef INPUT_DRIVER_H
#define INPUT_DRIVER_H

#include "libs/lvgl/lvgl.h"
#include "gesture.h"

/* =========================================================================
 * Gesture callback
 * ========================================================================= */

/**
 * @brief Called whenever a gesture is detected.
 *
 * @param event  Pointer to the completed gesture (valid only during callback).
 */
typedef void (*gesture_cb_t)(const gesture_event_t *event);

/* =========================================================================
 * API
 * ========================================================================= */

/**
 * @brief Initialise the input driver and register it with LVGL.
 *
 * @param display  The LVGL display returned by display_driver_init().
 *                 Used to link the indev to the correct display.
 *
 * Call after lv_init() and display_driver_init().
 */
void input_driver_init(lv_display_t *display);

/**
 * @brief Register an application-level gesture callback.
 *
 * Only one callback is supported at a time.  Call again to replace it.
 * Pass NULL to unregister.
 *
 * @param cb  Function to call when a gesture event is detected.
 */
void input_driver_set_gesture_cb(gesture_cb_t cb);

#endif /* INPUT_DRIVER_H */
