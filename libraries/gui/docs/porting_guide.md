# LVGL GUI Framework — Porting & Developer Guide

**Project:** ICDeC RISC-V Wearable
**GUI stack:** LVGL v9 · SDL2 (Linux dev) · RISC-V bare-metal (target)

---

## 1. Architecture overview

```
┌────────────────────────────────────────────────────────────┐
│  Application  (main.c)                                     │
│  • UI screens built with LVGL widgets                      │
│  • on_gesture() callback for navigation                    │
├──────────────────┬─────────────────────────────────────────┤
│  display_driver  │  input_driver                           │
│  (src/)          │  (src/)                                 │
│                  │                                         │
│  Linux:          │  Linux:                                 │
│  lv_sdl_window   │  lv_sdl_mouse  ─► gesture engine       │
│                  │                                         │
│  RISC-V:         │  RISC-V:                                │
│  OLED module ◄── │  touch module ─► gesture engine        │
│  flush_cb        │  read_cb                                │
├──────────────────┴─────────────────────────────────────────┤
│  gesture.c  (src/)  — pure C, zero LVGL dependency        │
│  click · double-click · long-press · swipe ×4             │
├────────────────────────────────────────────────────────────┤
│  LVGL v9  (libs/lvgl/)                                     │
├─────────────────────┬──────────────────────────────────────┤
│  Linux / SDL2       │  RISC-V / pulp-runtime               │
└─────────────────────┴──────────────────────────────────────┘
```

Everything above the `display_driver` / `input_driver` line is
platform-independent and never changes between builds.

---

## 2. Project structure

```
gui/
├── setup.sh               One-shot environment setup
├── lv_conf.h          LVGL v9 config (platform-aware via #ifdef)
├── main.c             Entry point + UI screens
├── Makefile           PLATFORM=linux | riscv
├── libs/
│   └── lvgl/          LVGL v9 source (cloned by setup.sh)
├── src/
│   ├── display_driver.h / .c   Platform display init
│   ├── input_driver.h  / .c   Platform input init + gesture bridge
│   ├── gesture.h       / .c   Pure-C gesture state machine
├── tests/
│   ├── test_gesture.c  Unit tests (host, no hardware needed)
│   └── Makefile
└── docs/
    └── porting_guide.md   ← this file
```

---

## 3. Quick start

### 3.1 First-time setup

```bash
# Install Linux prerequisites
sudo apt install build-essential libsdl2-dev git     # Ubuntu/Debian

# Run setup
./setup.sh
```

### 3.2 Build and run on Linux (SDL2 window)

```bash
make PLATFORM=linux
./build/gui_app
```

You will see a **512 × 256 px SDL window** (4× zoom of the 128 × 64 OLED).  
Mouse interaction:

| Mouse action | Gesture produced |
|---|---|
| Left-click and release | CLICK |
| Two quick left-clicks | DOUBLE_CLICK |
| Click and hold (600 ms) | LONG_PRESS |
| Click and drag ≥ 20 px horizontal | SWIPE_LEFT / SWIPE_RIGHT |
| Click and drag ≥ 20 px vertical | SWIPE_UP / SWIPE_DOWN |

### 3.3 Run unit tests (no hardware, no SDL)

```bash
cd app/tests
make
# Expected: 29 passed, 0 failed, exit code 0
```

### 3.4 Build for RISC-V

```bash
. ./env.sh                        # configure toolchain (once per terminal)
cd app
make PLATFORM=riscv
echo $?                           # must print 0
# binary: build/gui_app/gui_app
```

---

## 4. Connecting the ICDeC hardware modules

To add OLED and touch modules, plug them in
at **two clearly marked TODO locations**.

### 4.1 Display — `src/display_driver.c` (RISC-V section)

Find the `oled_flush_cb` function and `display_driver_init`:

```c
/* Step 1: include the OLED module header */
#include "oled.h"   /* or whatever name the team uses */

/* Step 2: initialise in display_driver_init() */
if (oled_init() != 0) { ... }

/* Step 3: in oled_flush_cb(), replace the stub printf with */
oled_flush(area->x1, area->y1, area->x2, area->y2, px_map);
lv_display_flush_ready(disp);   /* always keep this line */
```

The `px_map` buffer contains 16-bit RGB565 pixels (see `lv_conf.h`).
If the OLED module expects a 1-bit packed buffer, convert here:

```c
// Convert RGB565 → 1-bit: any non-zero pixel = ON
uint8_t oled_buf[DISPLAY_HOR_RES * DISPLAY_VER_RES / 8] = {0};
int n = lv_area_get_width(area) * lv_area_get_height(area);
uint16_t *src = (uint16_t *)px_map;
for (int i = 0; i < n; i++) {
    if (src[i]) oled_buf[i / 8] |= (1 << (i % 8));
}
oled_flush(area->x1, area->y1, area->x2, area->y2, oled_buf);
```

### 4.2 Touch input — `src/input_driver.c` (RISC-V section)

Find `touch_read_cb` and `input_driver_init`:

```c
/* Step 1: include the touch module header */
#include "touch.h"

/* Step 2: initialise in input_driver_init() */
if (touch_init() != 0) { ... }

/* Step 3: in touch_read_cb(), replace the stub with */
touch_point_t pt;
bool pressed = touch_read(&pt);
data->point.x = pt.x;
data->point.y = pt.y;
data->state   = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
```

No changes needed to the gesture engine or anywhere else in the stack.

---

## 5. Gesture engine reference (`src/gesture.h`)

### Gesture types

| Constant | Trigger |
|---|---|
| `GESTURE_CLICK` | Press + release, < 300 ms, drift < 10 px |
| `GESTURE_DOUBLE_CLICK` | Two clicks within 400 ms |
| `GESTURE_LONG_PRESS` | Held ≥ 600 ms with drift < 10 px |
| `GESTURE_SWIPE_LEFT` | Horizontal drag > 20 px, rightward |
| `GESTURE_SWIPE_RIGHT` | Horizontal drag > 20 px, leftward |
| `GESTURE_SWIPE_UP` | Vertical drag > 20 px, upward |
| `GESTURE_SWIPE_DOWN` | Vertical drag > 20 px, downward |

### Tuning thresholds

All thresholds are `#define` constants in `src/gesture.h`:

| Constant | Default | Adjust when… |
|---|---|---|
| `GESTURE_CLICK_MAX_MS` | 300 ms | Taps are missed → increase |
| `GESTURE_LONG_PRESS_MS` | 600 ms | Long press feels slow → decrease |
| `GESTURE_DCLICK_WINDOW_MS` | 400 ms | Double-click too sensitive → decrease |
| `GESTURE_CLICK_MAX_PX` | 10 px | Shaky panel misclassifies as swipe → increase |
| `GESTURE_SWIPE_MIN_PX` | 20 px | Swipes not detected → decrease |

After changing thresholds, re-run `cd app/tests && make` to verify all 29 tests still pass.

### Receiving gesture events (application level)

```c
#include "src/input_driver.h"

static void my_gesture_handler(const gesture_event_t *ev) {
    switch (ev->type) {
        case GESTURE_SWIPE_LEFT:   /* navigate to next screen */ break;
        case GESTURE_SWIPE_RIGHT:  /* navigate to prev screen */ break;
        case GESTURE_CLICK:        /* select focused element  */ break;
        case GESTURE_DOUBLE_CLICK: /* wake / toggle backlight */ break;
        case GESTURE_LONG_PRESS:   /* open context menu       */ break;
        default: break;
    }
}

// Register once after input_driver_init()
input_driver_set_gesture_cb(my_gesture_handler);
```

---

## 6. Adding sensor data to the UI

Other team members integrate by calling LVGL label update functions:

```c
// From the RTC task:
lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", h, m, s);

// From the HR/SpO2 task:
lv_label_set_text_fmt(lbl_hr,   "HR: %d bpm", bpm);
lv_label_set_text_fmt(lbl_spo2, "SpO2: %d%%",  spo2);

// From the step counter / activity task:
lv_label_set_text_fmt(lbl_steps, "Steps: %lu", steps);
lv_label_set_text_fmt(lbl_act,   "Activity: %s", name);
```

LVGL batches all updates and only redraws when `lv_timer_handler()` runs in
the main loop — there is no risk of partial-update visual glitches.

---

## 7. Platform selection reference

| Flag | Set by | Effect |
|---|---|---|
| `PLATFORM_LINUX` | `make PLATFORM=linux` or `-DPLATFORM_LINUX` | SDL2 display + mouse input |
| `PLATFORM_RISCV` | `make PLATFORM=riscv` or `-DPLATFORM_RISCV` | OLED flush_cb + touch read_cb |

`lv_conf.h` reads these flags via the C preprocessor, so a single file
configures both builds correctly.

---

## 8. Known limitations / future work

| Item | Status | Notes |
|---|---|---|
| Tick source (RISC-V) | TODO | Replace busy-loop in `main.c` with a 1 ms hardware timer ISR calling `lv_tick_inc(1)` |
| OLED flush_cb | Stub | Connect ICDeC OLED module (Section 4.1) |
| Touch read_cb | Stub | Connect ICDeC touch module (Section 4.2) |
| gesture_tick_ms (RISC-V) | g_tick_ms | Should use BSP timer when available |
| Multi-touch | Not supported | Only single-finger gestures |
| Screen brightness | Not implemented | Wire OLED brightness to a GPIO/PWM line |
