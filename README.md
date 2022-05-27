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

