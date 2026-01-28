# Repository structure

The `dtcore` repository is organized to keep the portable core library isolated from platform-specific build and runtime concerns.

## Top-level layout

| Path | Description |
|------|-------------|
| `dtcore_library/` | Core portable C library |
| `dtcore_demos/` | Demo implementations |
| `dtcore_tests/` | Unit tests |
| `apps_linux/` | Application builds for Linux |
| `apps_pico/` | Application builds for Raspberry Pi Pico (Pico SDK) |
| `apps_zephyr/` | Application builds for Zephyr |
| `apps_espidf/` | Application builds for ESP-IDF (FreeRTOS) |

## Design intent

- The core library contains **no platform conditionals**
- Platform-specific concerns are isolated to adapter layers
- Each `apps_*` directory represents a complete, runnable build
- Demos and tests exercise the same portable code paths

This structure allows the same modules to be compiled, tested, and demonstrated across host and embedded targets with minimal friction.