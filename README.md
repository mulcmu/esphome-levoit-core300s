# esphome-levoit-core300s
Work in progress for upgrading a Levoit Core 300s air purifier to use esphome local control.  The stock PCB has an ESP32-SOLO-1C with headers broken out to program.  All of the user interface and control seems to be implemented on the U2 MCU.  There is another power PCB that supplies 5V from mains input and connections for the 2.5PM sensor and Motor PWM control.

U2 Pins 13/14 are serial RX/TX connection to the ESP32 GPIO16/17.  Level shifting mosfets on bottom of PCB.  TP10 and TP33 by the ESP32 are respective test points.

Added a wire harness out the top with connections to the J1 header for the ESP32 and to TP10 and TP33.  Push button added for boot mode.  esptool connected to the Core300s ESP okay.  Using an ESP32 devboard to capture both sides of the core300s traffic on TP10 and TP33 with a little Arduino [project](https://github.com/mulcmu/ESP32_dual_serial_log).  115200 8n1 seemed to capture traffic okay.

Uploaded packet captures.  Working to decode.  Looks like a periodic status packet from MCU to ESP.  ESP has different command packets for different actions.  MCU to ESP has a timer feedback packet.  Long running packet capture to try to get filter percent change.

#### Packet structure:

`A5` appears to be start byte of packet

`22` send message or `12` ack message

`1-byte` sequence number, increments by one each packet

`1-byte` size of payload after checksum byte

`0` always zero

`1-byte` checksum.  Computed as 255 - ( (sum all of bytes except checksum) % 256 )

Payload:  MCU to ESP starts with `1 30 40`  ESP to MCU starts with `1 60 A2`

The acknowledge packet `12` contains the same sequence number, with the payload as `1 30 40` or `1 60 A2` and `0`

The MCU sends a status packet a few times a second to once every few seconds.  Frequency increases when controls are operated.  After firmware update the MCU was way less chatty.  Only sending updates on state changes.

`A5 22 1D 16 00 E4 01 30 40 00 07 00 02 01 00 01 64 01 00 00 01 03 00 00 00 3B 01 00`

MCU sends a packet when timer is running with remaining time

`A5 12 27 0C 00 DA 01 65 A2 00 08 0D 00 00 10 0E 00 00`

0D08 remaining seconds

#### Payload structure:

22 byte long (0x16) status packet payload:

Byte 8 power 0x01 On 0x00 off

Byte 9 fan mode sleep 0x01, auto 0x02 On, 0x00 manual

Byte 10 fan speed 0x01, 0x02, 0x03

Byte 11 Screen 0 lights off, 0x64 lights on, Sleep mode

Byte 12 Screen light off 0x00, Lights on 0x01

Byte 16 PM2.5???

Byte 18 Display Lock 0x00 unlocked,  0x01 locked

Byte 19 Fan Auto Mode 0x00 default/air quality, 0x01 Quiet,  0x02 Efficient

Byte 20 & 21 Efficient Area `3B 01` 100 sq ft   

#### ESPHome functions:

- Toggle Power
- Set fan Mode, Manual, Sleep, Auto
- Set fan speed for manual mode
- PM2.5 sensor feedback
- Filter life remaining
- No Timer/Schedule support (implement in HA instead of on device)

#### TODO:

- Decode serial protocol / payload struture
- PM2.5 sensor is reading super low.  Other 3 units consistent 0 to 3 ug/m3.  Burning plastic triggered spike.
- Code custom UART esphome interface.
- Investigate OTA of stock hardware without disassembly.

#### Notes:

Vesync app has original FW version for ESP and MCU from the PCB sticker.  OTA firmware updated ESP and MCU firmware.
