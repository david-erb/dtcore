# Quick Start (Linux)

This is the fastest way to verify that dtcore builds and runs on a desktop system.

## Build and run `demo_hello`

```bash
git clone https://github.com/david-erb/dtcore.git
cd dtcore/apps_linux/demo_hello
rm -rf build 
cmake -S . -B build
cmake --build build 
# run the executable
./build/app
```

If all goes well, you'll see this line of output:  

`[INFO ] demo_hello: Hello from dtcore_demo_hello!`