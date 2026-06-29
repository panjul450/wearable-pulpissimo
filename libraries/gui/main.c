/**
 * @file    main.c
 * @brief   NusaCore RISC-V Wearable — GUI Framework Entry Point
 *
 * Build targets:
 *   make PLATFORM=linux    SDL2 window for development on Ubuntu/CachyOS
 *   make PLATFORM=riscv    pulp-runtime on NusaCore board
 */

/* ── Standard library includes MUST be at the top, outside any function ── */
#include <stdio.h>
#include <stdint.h>

#ifdef PLATFORM_LINUX
#  include <unistd.h>   /* usleep()  */
#endif

/* ── LVGL & project headers ─────────────────────────────────────────────── */
#include "libs/lvgl/lvgl.h"
#include "src/display_driver.h"
#include "src/input_driver.h"

/* =========================================================================
 * RISC-V tick counter
 * Incremented manually in the main loop (replace with hardware timer ISR).
 * On Linux, usleep() provides the timing and we just call lv_tick_inc().
 * ========================================================================= */
#ifndef PLATFORM_LINUX
volatile uint32_t g_tick_ms = 0;
#endif

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
static void ui_create(void);
static void on_gesture(const gesture_event_t *event);

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void)
{
    printf("\n");
    printf("================================================\n");
    printf("  NusaCore GUI Framework\n");
    printf("  ICDeC RISC-V Wearable Project\n");
#ifdef PLATFORM_LINUX
    printf("  Platform : Linux / SDL2 (development)\n");
#else
    printf("  Platform : RISC-V bare-metal (NusaCore)\n");
#endif
    printf("================================================\n\n");

    /* 1. Initialise LVGL */
    lv_init();
    printf("[MAIN] LVGL initialised\n");

    /* 2. Display (SDL2 window on Linux, OLED on RISC-V) */
    lv_display_t *display = display_driver_init();
    if (display == NULL) {
        fprintf(stderr, "[MAIN] FATAL: display_driver_init() failed\n");
        return 1;
    }

    /* 3. Input (SDL mouse on Linux, touch controller on RISC-V) */
    input_driver_init(display);

    /* 4. Register application gesture handler */
    input_driver_set_gesture_cb(on_gesture);

    /* 5. Build the UI */
    ui_create();
    printf("[MAIN] UI ready — entering event loop\n\n");

    /* 6. Event loop */
    while (1) {
#ifdef PLATFORM_LINUX
        lv_tick_inc(5);
        lv_timer_handler();
        usleep(5000);            /* 5 ms sleep keeps CPU usage low      */
#else
        lv_tick_inc(1);
        g_tick_ms++;
        lv_timer_handler();
        /* TODO: replace busy-loop with 1 ms hardware timer ISR         */
#endif
    }

    return 0;
}

/* =========================================================================
 * UI — screen creation
 *
 * Three-tile tileview; swipe left/right to navigate.
 *   Tile 0  Clock   — time & date
 *   Tile 1  Health  — heart rate + SpO2
 *   Tile 2  Fitness — steps + activity
 *
 * See docs/extending.md → "Adding a new UI screen" to add more tiles.
 * =========================================================================
 */
static lv_obj_t *g_tileview  = NULL;

/* Per-screen label handles so sensor tasks can call lv_label_set_text() */
static lv_obj_t *g_lbl_time  = NULL;
static lv_obj_t *g_lbl_date  = NULL;
static lv_obj_t *g_lbl_hr    = NULL;
static lv_obj_t *g_lbl_spo2  = NULL;
static lv_obj_t *g_lbl_steps = NULL;
static lv_obj_t *g_lbl_act   = NULL;

static void ui_create(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    g_tileview = lv_tileview_create(scr);
    lv_obj_set_size(g_tileview, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(g_tileview, lv_color_black(), 0);

    /* ── Tile 0: Clock ──────────────────────────────────────────── */
    lv_obj_t *t0 = lv_tileview_add_tile(g_tileview, 0, 0, LV_DIR_RIGHT);
    lv_obj_set_style_bg_color(t0, lv_color_black(), 0);

    g_lbl_time = lv_label_create(t0);
    lv_label_set_text(g_lbl_time, "00:00:00");
    lv_obj_set_style_text_color(g_lbl_time, lv_color_white(), 0);
    lv_obj_align(g_lbl_time, LV_ALIGN_CENTER, 0, -10);

    g_lbl_date = lv_label_create(t0);
    lv_label_set_text(g_lbl_date, "01/01/2025");
    lv_obj_set_style_text_color(g_lbl_date, lv_color_white(), 0);
    lv_obj_align(g_lbl_date, LV_ALIGN_CENTER, 0, 8);

    /* ── Tile 1: Health ─────────────────────────────────────────── */
    lv_obj_t *t1 = lv_tileview_add_tile(g_tileview, 1, 0,
                                         LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_set_style_bg_color(t1, lv_color_black(), 0);

    lv_obj_t *hdr1 = lv_label_create(t1);
    lv_label_set_text(hdr1, "Health");
    lv_obj_set_style_text_color(hdr1, lv_color_white(), 0);
    lv_obj_align(hdr1, LV_ALIGN_TOP_MID, 0, 4);

    g_lbl_hr = lv_label_create(t1);
    lv_label_set_text(g_lbl_hr, "HR: -- bpm");
    lv_obj_set_style_text_color(g_lbl_hr, lv_color_white(), 0);
    lv_obj_align(g_lbl_hr, LV_ALIGN_CENTER, 0, -8);

    g_lbl_spo2 = lv_label_create(t1);
    lv_label_set_text(g_lbl_spo2, "SpO2: --%");
    lv_obj_set_style_text_color(g_lbl_spo2, lv_color_white(), 0);
    lv_obj_align(g_lbl_spo2, LV_ALIGN_CENTER, 0, 8);

    /* ── Tile 2: Fitness ────────────────────────────────────────── */
    lv_obj_t *t2 = lv_tileview_add_tile(g_tileview, 2, 0, LV_DIR_LEFT);
    lv_obj_set_style_bg_color(t2, lv_color_black(), 0);

    lv_obj_t *hdr2 = lv_label_create(t2);
    lv_label_set_text(hdr2, "Fitness");
    lv_obj_set_style_text_color(hdr2, lv_color_white(), 0);
    lv_obj_align(hdr2, LV_ALIGN_TOP_MID, 0, 4);

    g_lbl_steps = lv_label_create(t2);
    lv_label_set_text(g_lbl_steps, "Steps: 0");
    lv_obj_set_style_text_color(g_lbl_steps, lv_color_white(), 0);
    lv_obj_align(g_lbl_steps, LV_ALIGN_CENTER, 0, -8);

    g_lbl_act = lv_label_create(t2);
    lv_label_set_text(g_lbl_act, "Activity: Idle");
    lv_obj_set_style_text_color(g_lbl_act, lv_color_white(), 0);
    lv_obj_align(g_lbl_act, LV_ALIGN_CENTER, 0, 8);
}

/* =========================================================================
 * Gesture handler
 * Called by input_driver when a gesture is recognised.
 * ========================================================================= */
static int g_current_tile = 0;
#define TILE_COUNT 3

static void on_gesture(const gesture_event_t *event)
{
    switch (event->type) {
        case GESTURE_SWIPE_LEFT:
            if (g_current_tile < TILE_COUNT - 1) {
                g_current_tile++;
                lv_obj_set_tile_id(g_tileview,
                                   (uint32_t)g_current_tile, 0,
                                   LV_ANIM_ON);
                printf("[APP] → tile %d\n", g_current_tile);
            }
            break;

        case GESTURE_SWIPE_RIGHT:
            if (g_current_tile > 0) {
                g_current_tile--;
                lv_obj_set_tile_id(g_tileview,
                                   (uint32_t)g_current_tile, 0,
                                   LV_ANIM_ON);
                printf("[APP] ← tile %d\n", g_current_tile);
            }
            break;

        case GESTURE_CLICK:
            printf("[APP] Click at (%d, %d)\n",
                   event->end_x, event->end_y);
            break;

        case GESTURE_DOUBLE_CLICK:
            printf("[APP] Double-click — toggle backlight\n");
            break;

        case GESTURE_LONG_PRESS:
            printf("[APP] Long press — open settings\n");
            break;

        default:
            break;
    }
}

/* =========================================================================
 * pulp-runtime hook (RISC-V only)
 * ========================================================================= */
void pe_start(void)
{
    /* Reserved for future multi-core use on NusaCore */
}
