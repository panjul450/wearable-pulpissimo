/**
 * @file    test_gesture.c
 * @brief   Unit tests for the gesture recognition engine
 *
 * Compile & run on any Linux host (no SDL2, no LVGL, no hardware):
 *
 *   cd app/tests
 *   make
 *
 * Expected: all PASS, exit code 0.
 *
 * Technique: gesture.c calls gesture_tick_ms() which we define here
 * as a simulated clock.  This lets us control timing precisely.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Include the gesture engine directly (no LVGL dependency) */
#include "../src/gesture.h"

/* =========================================================================
 * Simulated clock — overrides the real gesture_tick_ms() defined in
 * input_driver.c.  Here we implement it ourselves for testing.
 * ========================================================================= */
static uint32_t g_sim_ms = 0;

uint32_t gesture_tick_ms(void) { return g_sim_ms; }
static void tick(uint32_t ms)  { g_sim_ms += ms;  }

/* =========================================================================
 * Mini test framework
 * ========================================================================= */
static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg)                                                   \
    do {                                                                     \
        if (cond) { printf("  PASS: %s\n", msg); g_pass++;                  \
        } else    { printf("  FAIL: %s  (line %d)\n", msg, __LINE__);       \
                    g_fail++; }                                               \
    } while (0)

/* =========================================================================
 * Helpers: simulate touch contacts
 * ========================================================================= */

/** Simulate a touch press+hold+release. */
static void touch(int16_t sx, int16_t sy,
                  uint32_t hold_ms,
                  int16_t ex, int16_t ey)
{
    gesture_feed(true,  sx, sy);
    tick(hold_ms / 2);
    gesture_feed(true,  ex, ey);   /* Simulate slow movement mid-contact */
    tick(hold_ms - hold_ms / 2);
    gesture_feed(false, ex, ey);
}

/** Drain any pending event and return it. */
static gesture_event_t drain(void)
{
    gesture_event_t ev = {0};
    gesture_poll(&ev);
    return ev;
}

/* =========================================================================
 * Test cases
 * ========================================================================= */

static void test_idle_no_event(void)
{
    printf("\n[test_idle_no_event]\n");
    gesture_init();
    g_sim_ms = 0;

    gesture_event_t ev;
    ASSERT(!gesture_poll(&ev), "no event when idle");
}

static void test_click(void)
{
    printf("\n[test_click]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(64, 32, 100, 64, 32);
    tick(GESTURE_DCLICK_WINDOW_MS + 1); /* Let double-click window expire */
    gesture_feed(false, 0, 0);          /* One more tick to run expiry logic */

    gesture_event_t ev;
    bool got = gesture_poll(&ev);

    ASSERT(got,                      "event received");
    ASSERT(ev.type == GESTURE_CLICK, "type == CLICK");
    ASSERT(ev.start_x == 64,        "start_x correct");
    ASSERT(ev.start_y == 32,        "start_y correct");
    ASSERT(ev.duration_ms <= 110,   "duration within range");
}

static void test_double_click(void)
{
    printf("\n[test_double_click]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(64, 32, 80, 64, 32);         /* First tap */
    tick(150);                          /* Gap < GESTURE_DCLICK_WINDOW_MS */
    touch(64, 32, 80, 64, 32);         /* Second tap */
    tick(10);

    gesture_event_t ev;
    bool got = gesture_poll(&ev);

    ASSERT(got,                              "event received");
    ASSERT(ev.type == GESTURE_DOUBLE_CLICK,  "type == DOUBLE_CLICK");
}

static void test_double_click_window_expired(void)
{
    printf("\n[test_double_click_window_expired]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(64, 32, 80, 64, 32);
    tick(GESTURE_DCLICK_WINDOW_MS + 100);  /* Window expires */
    gesture_feed(false, 0, 0);             /* Trigger expiry in gesture_poll */

    gesture_event_t ev;
    bool got = gesture_poll(&ev);

    ASSERT(got,                      "event received");
    ASSERT(ev.type == GESTURE_CLICK, "type == CLICK (not double, window expired)");
}

static void test_long_press(void)
{
    printf("\n[test_long_press]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(64, 32, GESTURE_LONG_PRESS_MS + 200, 65, 32);

    gesture_event_t ev;
    bool got = gesture_poll(&ev);

    ASSERT(got,                            "event received");
    ASSERT(ev.type == GESTURE_LONG_PRESS,  "type == LONG_PRESS");
    ASSERT(ev.duration_ms >= GESTURE_LONG_PRESS_MS, "duration >= threshold");
}

static void test_swipe_left(void)
{
    printf("\n[test_swipe_left]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(100, 32, 150, 30, 32);   /* Move 70 px left */

    gesture_event_t ev = drain();
    ASSERT(ev.type == GESTURE_SWIPE_LEFT,         "type == SWIPE_LEFT");
    ASSERT(ev.delta_x < -GESTURE_SWIPE_MIN_PX,   "delta_x negative");
}

static void test_swipe_right(void)
{
    printf("\n[test_swipe_right]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(20, 32, 150, 100, 32);

    gesture_event_t ev = drain();
    ASSERT(ev.type == GESTURE_SWIPE_RIGHT,        "type == SWIPE_RIGHT");
    ASSERT(ev.delta_x > GESTURE_SWIPE_MIN_PX,    "delta_x positive");
}

static void test_swipe_up(void)
{
    printf("\n[test_swipe_up]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(64, 55, 150, 64, 5);

    gesture_event_t ev = drain();
    ASSERT(ev.type == GESTURE_SWIPE_UP, "type == SWIPE_UP");
}

static void test_swipe_down(void)
{
    printf("\n[test_swipe_down]\n");
    gesture_init();
    g_sim_ms = 0;

    touch(64, 5, 150, 64, 55);

    gesture_event_t ev = drain();
    ASSERT(ev.type == GESTURE_SWIPE_DOWN, "type == SWIPE_DOWN");
}

static void test_click_tiny_drift(void)
{
    printf("\n[test_click_tiny_drift]\n");
    gesture_init();
    g_sim_ms = 0;

    /* 5 px drift — within GESTURE_CLICK_MAX_PX → still a click */
    touch(64, 32, 100, 69, 32);
    tick(GESTURE_DCLICK_WINDOW_MS + 1);
    gesture_feed(false, 0, 0);

    gesture_event_t ev;
    bool got = gesture_poll(&ev);
    ASSERT(got,                      "event received");
    ASSERT(ev.type == GESTURE_CLICK, "small drift still CLICK");
}

static void test_type_str(void)
{
    printf("\n[test_type_str]\n");
    ASSERT(strcmp(gesture_type_str(GESTURE_NONE),         "NONE")         == 0, "NONE");
    ASSERT(strcmp(gesture_type_str(GESTURE_CLICK),        "CLICK")        == 0, "CLICK");
    ASSERT(strcmp(gesture_type_str(GESTURE_DOUBLE_CLICK), "DOUBLE_CLICK") == 0, "DOUBLE_CLICK");
    ASSERT(strcmp(gesture_type_str(GESTURE_LONG_PRESS),   "LONG_PRESS")   == 0, "LONG_PRESS");
    ASSERT(strcmp(gesture_type_str(GESTURE_SWIPE_LEFT),   "SWIPE_LEFT")   == 0, "SWIPE_LEFT");
    ASSERT(strcmp(gesture_type_str(GESTURE_SWIPE_RIGHT),  "SWIPE_RIGHT")  == 0, "SWIPE_RIGHT");
    ASSERT(strcmp(gesture_type_str(GESTURE_SWIPE_UP),     "SWIPE_UP")     == 0, "SWIPE_UP");
    ASSERT(strcmp(gesture_type_str(GESTURE_SWIPE_DOWN),   "SWIPE_DOWN")   == 0, "SWIPE_DOWN");
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void)
{
    printf("================================================\n");
    printf("  NusaCore Gesture Engine — Unit Tests\n");
    printf("================================================\n");

    test_idle_no_event();
    test_click();
    test_double_click();
    test_double_click_window_expired();
    test_long_press();
    test_swipe_left();
    test_swipe_right();
    test_swipe_up();
    test_swipe_down();
    test_click_tiny_drift();
    test_type_str();

    printf("\n================================================\n");
    printf("  %d passed, %d failed\n", g_pass, g_fail);
    printf("================================================\n");

    return g_fail == 0 ? 0 : 1;
}
