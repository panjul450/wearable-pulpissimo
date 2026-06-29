# LVGL GUI Framework

LVGL v9 GUI framework for the **RISC-V wearable** (ICDeC).
Runs in an SDL2 window on Linux for development, then targets the
PULPissimo board with an OLED display.

---

## Prerequisites

### Linux build (LVGL & SDL2 window)
```bash
sudo apt update && sudo apt install -y \
  build-essential \
  cmake \
  pkg-config \
  python3-pip \
  python3.12-venv \
  libwayland-dev \
  libxkbcommon-dev \
  libwayland-bin \
  wayland-protocols \
  libsdl2-dev 
```

### RISC-V build
You need two things:
- `chroot.tar.xz` — RISC-V toolchain (`riscv32-unknown-elf-gcc`)
- `pulp-runtime` github clone

---

## First-time setup

```bash
git clone <this-repo-url>
cd libraries/gui

./setup.sh
```

---

## Building

### Linux — SDL2 window
```bash
make PLATFORM=linux
./build/gui_app
```
A **512 × 256 px SDL window** opens (4× zoom of the 128 × 64 OLED).

| Mouse action | Gesture |
|---|---|
| Left-click + release | CLICK |
| Two quick clicks | DOUBLE_CLICK |
| Hold 600 ms | LONG_PRESS |
| Drag ≥ 20 px horizontal | SWIPE_LEFT / RIGHT |
| Drag ≥ 20 px vertical | SWIPE_UP / DOWN |

### Unit tests — no hardware, no SDL
```bash
cd tests
make
# Expected: 29 passed, 0 failed, exit code 0
```

### RISC-V — NusaCore board
```bash
# Verify the toolchain was found:
riscv32-unknown-elf-gcc --version

# Build:
make PLATFORM=riscv
echo $?     # must print 0

# Binary: app/build/gui_app/
```

---

## Project structure

```
gui/
├── setup.sh             First-time environment setup
├── lv_conf.h        LVGL v9 config (platform-aware)
├── main.c           Entry point + UI screens
├── Makefile         PLATFORM=linux | riscv
├── libs/
│   └── lvgl/        LVGL v9 — git submodule, not uploaded
├── src/
│   ├── display_driver.{h,c}   SDL2 ↔ OLED abstraction
│   ├── input_driver.{h,c}     SDL mouse ↔ touch abstraction
│   └── gesture.{h,c}          Click / double-click / swipe engine
├── tests/
│       ├── test_gesture.c
│       └── Makefile
└── docs/
    ├── porting_guide.md   Platform porting + build instructions
    └── extending.md       Adding inputs, outputs, and UI screens
```

---

## LVGL as a dependency

LVGL is **not uploaded to this repository**. It is managed as a git
submodule pointing to the official `release/v9.2` branch.

```bash
# Clone with LVGL in one command:
git clone --recurse-submodules <this-repo-url>

# Or, after a plain clone:
git submodule update --init
```

To update LVGL to a newer v9.x patch:
```bash
cd libs/lvgl
git fetch origin
git checkout release/v9.2   # or a specific tag
cd ../..
git add app/libs/lvgl
git commit -m "chore: bump LVGL to v9.2.x"
```

---

## Connecting hardware modules

| What to connect | Where in the code |
|---|---|
| ICDeC OLED module | `src/display_driver.c` → `oled_flush_cb()` TODO |
| ICDeC touch module | `src/input_driver.c` → `touch_read_cb()` TODO |
| Sending sensor data to UI | Call `lv_label_set_text_fmt()` on the label handles in `main.c` |

See `docs/extending.md` for step-by-step instructions and complete examples.

---

## Documentation

| File | Contents |
|---|---|
| `docs/porting_guide.md` | Build setup, environment, platform porting details |
| `docs/extending.md` | How to add new displays, inputs, and UI screens |
