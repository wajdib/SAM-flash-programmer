# SAM-flash-programmer
Atmel's SAM-ARM flasher utility, based on BOSSA.

Simple to use flash programmer written in C++, using QT, and BOSSA.

![alt text](https://github.com/wajdib/SAM-flash-programmer/blob/master/updater1.PNG "")
![alt text](https://github.com/wajdib/SAM-flash-programmer/blob/master/updater3.PNG "")

# Purpose

The purpose of this tool is to be able to upload binary flash files using USB serial port easily without the need of a compiler/IDE 
The tool will auto detect serial ports, let's you select binary file, and flash using BOSSA* utility.

Can be used to upload Arduino compiled sketches 

# Supported Device Families
* SAM7S
* SAM7SE
* SAM7X
* SAM7XC
* SAM3N
* SAM3S
* SAM3U
* SAM4E
* SAM4S
* SAM3X*
* SAM3A*
* SAM7L*
* SAM9XE*
* SAMD21*

*BOSSA is released under BSD 3-Clause.
