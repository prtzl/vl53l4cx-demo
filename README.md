# vl53l4cx Time-of-Flight distance sensor - demo

## Dependencies used

```
STM32CubeMX v6.2.1 - generated STM32CubeIDE (v1.8.0) project
arm-none-eabi-gcc v10.3.0
gnumake v4.3
cmake v3.22.3
st-flash v1.7.0 (for flashing)
nix v2.8.1 (optional)
```

## Build

Run `make -j` to compile the project, executable will be in `./build`.  
Run `make flash` to flash with `st-flash`.  
With nix run `nix build`, executable will be in `./result/bin`.
To flash with nix (and (re)build), run `nix run .\#flash-st.x86_64-linux`

## What did I do?

I downloaded the example from ST's [website](https://www.st.com/content/st_com/en/products/embedded-software/imaging-software/stsw-img029.html). As of time of testing, it was version `1.2.8`. It is configured for a few nucleo boards along with the [shield](https://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/imaging-evaluation-boards/x-nucleo-53l4a2.html#tools-software), which I'm using as well with one sensor only (center). Also I'm using `STM32F411RE` nucleo board.  

I loaded up the example, which is provided for CubeIDE, but it didn't compile. Some includes were faulty - fix the capitalization to lower caps and add an include. It seems like typo or a refactor fragment.  
Then I navigated the example `main.c` and found that I could loose all the "platform" code, that does a bunch of system init along with some other ICs, that I don't have in my system. Specifically I had a problem with a reset function for toggling the reset pin of the IC, which tried to do that via a "i2c expander IC", which I don't have ... so don't use that code.  

Then I just applied my code style, removed the interrupt example (pin and interrupt is still configured for the project, just not used) and ran `clang-format` over the entire source code. I modified the main loop to print just the good detections. I also removed all global variables for readability. I hope the comments help :).  

All the "driver" sources are located in `vl53l4cx` directory. This one was left mostly untouched (one include fix). All platform specific code was put in `Core/vl53l4cx` and has been shrank as much as possible without workings or major modifications. I also added `Core/Src/syscalls.c` along with `fputc` in `main.c` so that "printf" can be used over uart.  
