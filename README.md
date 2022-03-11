# esphome-levoit-core300s
Work in progress for upgrading a Levoit Core 300s air purifier to use esphome local control.  The stock PCB has an ESP32-SOLO-1C with headers broken out to program.  All of the user interface and control seems to be implemented on the U2 MCU.  There is another power PCB that supplies 5V from mains input and connections for the 2.5PM sensor and Motor PWM control.

U2 Pins 13/14 appear to be serial RX/TX connection to the ESP32 GPIO16/17.  Level shifting mosfets on bottom of PCB.  TP10 and TP33 by the ESP32 are respective test points.

Added a wire harness out the top with connections to the J1 header for the ESP32 and to TP10 and TP33.  It looks like 115200 baud serial.  esptool connected to the ESP okay.  115200 8n1 seemed to capture traffic okay.

#### Packet structure

`A5` appears to be start byte of packet

`22` send message or `12` ack message

`1-byte` sequence number, increments by one each packet

`1-byte` size of payload after checksum byte

`0` always zero

`1-byte` checksum.  Computed as 255 - ( (sum all of bytes except checksum) % 256 )

Payload:  MCU to ESP starts with `1 30 40`  ESP to MCU starts with `1 60 A2`

The acknowledge packet `12` contains the same sequence number, with the payload as `1 30 40` or `1 60 A2` and `0`

The MCU sends a status packet several times a second.

TODO:

- Decode serial protocol / payload struture
- Code custom UART esphome interface 
- Investigate OTA of stock hardware without disassembly
- Order a few more 300s for rest of house
