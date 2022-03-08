# esphome-levoit-core300s
Work in progress for upgrading a Levoit Core 300s air purifier to use esphome local control.  The stock PCB has an ESP32-SOLO-1C with headers broken out to program.  All of the user interface and control seems to be implemented on the U2 MCU.  There is another power PCB that supplies 5V from mains input and connections for the 2.5PM sensor and Motor PWM control.

U2 Pins 13/14 appear to be RX/TX connection to the ESP32.  Level shifting mosfets on bottom of PCB.

TODO:

- Wire up 5V external power supply to PCB.
- Make serial breakout adapter with flash push button.  Save stock ESP32 flash image.
- Capture traffic between U2 and ESP32
- Code custom UART esphome interface 
- Investigate OTA of stock hardware without disassembly
- Order a few more 300s for rest of house
