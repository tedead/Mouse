# Mouse

Human-like USB mouse jiggler for ATmega32U4 boards (Arduino Micro / Leonardo / Pro Micro).
Enumerates as a standard USB HID mouse and makes small, natural-looking cursor movements
at random intervals to keep the screen from locking.

## Hardware

Any board with **native USB HID**:

| Board | Chip | PlatformIO `board` |
|---|---|---|
| Arduino Micro | ATmega32U4 | `micro` |
| Arduino Leonardo | ATmega32U4 | `leonardo` |
| Pro Micro 5V/16MHz | ATmega32U4 | `sparkfun_promicro16` |

**Not supported:** Uno R3, Mega 2560, classic Nano, Nano Every. These use a USB-to-serial bridge
and fail with `'Mouse' was not declared in this scope`.

## Build & Upload

`platformio.ini`:

```ini
[env:micro]
platform = atmelavr
board = micro
framework = arduino
lib_deps = arduino-libraries/Mouse
```

```powershell
pio run -t upload
pio device monitor -b 115200   # optional: watch move log
```

If the upload can't find the port, double-tap reset just before upload starts. The 32U4 bootloader
stays active for about 8 seconds.

## Tuning

Constants at the top of `src/main.cpp`:

| Constant | Default | Purpose |
|---|---|---|
| `IDLE_MIN_MS` / `IDLE_MAX_MS` | 15000 / 45000 | Random idle between bursts. Keep the max below your OS lock timeout. |
| `DIST_MIN` / `DIST_MAX` | 8 / 60 | Move distance in HID counts. Actual pixels depend on OS pointer acceleration. |
| `STEP_MS` | 8 | Interval between HID reports (~125 Hz, typical mouse poll rate). |

## How the motion works

- **Bursts:** 1–3 moves with an 80–600 ms pause between them, then a long idle.
- **Velocity:** minimum-jerk profile (`10t³ − 15t⁴ + 6t⁵`), a smooth start and stop that models
  human reaching motion.
- **Path:** a quadratic Bézier arc with a random bend of ±30%.
- **Tremor:** ±0.5 count of noise per step. The final step lands exactly on the target.
- **Duration:** scales with distance (roughly following Fitts' law) plus random variation.
- **Drift correction:** each new target is biased 60% back toward the start, so the cursor stays put.
- **Seeding:** `randomSeed()` uses noise from a floating analog pin plus `micros()`, so each boot
  produces a different pattern.

## Notes

- The device appears in Device Manager as an *Arduino Micro* (VID `2341`).
- Unplug the board to stop it. There is no on/off switch.
