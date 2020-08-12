# 2D Input on any surface - Modular PCB

## Features
Modular version - Core
* ESP32-WROOM32 Module
* CP2104 USB-UART bridge
* 1S Li-Po charger
* 3.3V LDO
* 2x FPC breakout connectors

Modular version - Peripherals
* MPU9250 IMU
* Custom narrow angle IR sensor ( & )
* 2x buttons
* Capacitive touch sensor board (3 positions)
* Joystick

## Hardware connections

|Signal| Arduino Pin|Note|
|:---:|:---:|:---:|
|RXD0|RXD0|Programming|
|TXD0|TXD0|Programming|
|IMU_SDA|23|
|IMU_SCL|22|
|BUTTON 1 or CAP_TOUCH_1|14, A6||
|BUTTON 2|15||
|CAP_TOUCH_2 or JOY_X|33, A9||
|CAP_TOUCH_3 or JOY_Y|32, A7||
|LED|13||
|IR_LED|12||
|IR_SENSOR_OUT|34, A2|Analog|

## Programming
Follow normal Arduino for ESP32 instructions. Select "Adafruit feather" as Board in Arduino IDE to program.

## Errata
* MPU9250 is discontinued - don't use it for any new designs
* Problem: cannot connect to the MCU to flash it.   
    * Solution: Missing trace on GPIO0 line that connects to the auto-reset circuit. Used a rework wire as a temporary fix.

## References & tutorials
* [MPU4920 datasheet]()
* [Capacitive touch sensor design guide]()
