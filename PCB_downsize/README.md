# 2D Input on any surface - Downsized PCB

## Features
* NRF52 (BMD350 module)
* SWD Programming & debugging port
* 1S Li-Po charger
* 3.3V & 1.8V LDO
* MPU9250 IMU
* VCNL36687S Proximity sensor

## Hardware connections

|Signal|Pin|Note|
|:---:|:---:|:---:|
|RESET|21||
|INT_IMU|17||
|INT_IR|30||
|SDA|6||
|SCL|8||

## Debugging notes
* VCNL36687S soldered at 90 deg: one side pins soldered, other side using rework wire
* Add connector to external button in next version

## Programming
* Didn't get permission to share firmware
* [NRF52 dev with visual studio code](https://github.com/rayan4444/vs_code_setup_tutorials/tree/master/NRF52)

## Datasheets
* [BMD350](https://www.u-blox.com/sites/default/files/BMD-350_DataSheet_%28UBX-19033354%29.pdf)
* [MPU9250](https://invensense.tdk.com/wp-content/uploads/2015/02/PS-MPU-9250A-01-v1.1.pdf)
* [VCNL36687S](https://www.vishay.com/docs/84907/vcnl36687s.pdf)
