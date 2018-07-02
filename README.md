# K34M

Fork/based on https://github.com/ryannining/karyacontroller  a dedicated board and code for esp8266 and in future esp32

Using D1 mini from wemos and in future d1 mini esp32 from eht modules

Board for cover multiple Maker needs compatible with that gcode machines.

cnc,plotter, 3dprinter,plasma cutter,foam cutter..etc.

Why bigger size of original karyacontroller:
  - for improve wiring for no have cables on steppers or other cables on boad.
  - cooling use uniqe fan80mm for cold stepper and esp.
  - have space for eventual next updates  or for move on esp32 and 8 steppers
 
### board features:
  - Dimension of board 80x80  
  - 4 stepper max
  - 1 power output (with logical pwm near)
  - 1 thermoresistor 100k
  - 1 endstop line (pullup or pulldown)
  - 1 full external output pins
  - 1 configuration pins(for 4 stepper indipendent or dir E-Z in common)
  - mount holes for using standard unique 80mm

### Limits and alternative solution:


image of demo board
![Image of board](https://github.com/exilaus/K34M/blob/master/images/3d.JPG)

image of pins
![Image of board](https://github.com/exilaus/K34M/blob/master/images/board.jpg)

### Configuration pins
Full 4° Axis without no endstops
![Image of board](https://github.com/exilaus/K34M/blob/master/images/Picture1.jpg)
That configuration need for use 4° (E Driver) driver fully indipendent. For have that configuration need lose use of endstop.

That configuration are usefull for 4° axis Foam cutter cnc

4° Axis with endstops 

![Image of board](https://github.com/exilaus/K34M/blob/master/images/Picture2.jpg)
That configuration use E DIR in common with Z DIR that permit use board for CNC  3DPrinters plotter and so on

Tha most common configuration for any build.

Board currenlty in shippping to italy need wait for testing and we release gerber original easyeda project and cleaned code .
