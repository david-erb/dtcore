# Running dtcore applications

Althought dtcore is primarily a library, it comes with some ready-to-run demos and tests as applications.

This section describes how to build and run these applications across supported platforms.

## Application list

The dtcore repository contains a discrete set of runnable applications:

| application | summary |
| --- | --- |
| `demo_hello`   | minimal app |
| `demo_basics`  | basic functionality sweep |
| `test_all`     | full test runner |

Each application is built as a separate artifact on the following platforms:

- **Linux**: standalone executables
- **ESP-IDF**: firmware images
- **Zephyr**: firmware images
- **Raspberry Pi Pico**: firmware images


## How applications are compiled per platform

Hopefully you have your development environments set up already!

Using demo_hello as example...

### Linux

```bash
cd apps_linux/demo_hello
rm -rf build 
cmake -S . -B build
cmake --build build 
# run the executable
./build/app
```

### ESP-IDF

```bash
cd apps_espidf/demo_hello
rm -rf build 
idf.py set-target esp32
idf.py build
# run on the host
idf.py qemu
```

### nRF5340 with Zephyr

```bash
cd apps_zephyr/demo_hello
rm -rf build 
west build -b native_sim
# run in the simulator
build/demo_hello/zephyr/zephyr.exe
```


### Raspberry Pi Pico

```bash
cd apps_pico/demo_hello
rm -rf build 
cmake -S . -B build
cmake --build build 
# put the Pico into BOOTSEL and flash the executable
picotool load build/app.elf -f
# or something like this on windows
cp build/app.uf2 /drives/d
# on something like this on Linux
cp build/app.uf2 /media/$USER/RPI-RP2/
```


