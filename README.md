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

Payload Type:  3 bytes,  12 unique types found so far

The acknowledge packet `12` contains the same sequence number, with the `payload type` and `0`

#### Payload types:

##### `01 30 40` - Status 

The MCU sends a status packet a few times a second to once every few seconds.  Frequency increases when controls are operated.  After firmware update the MCU was way less chatty.  Only sending updates on state changes or once every 60 seconds.

`A5 22 1D 16 00 E4 01 30 40 00 07 00 02 01 00 01 64 01 00 00 01 03 00 00 00 3B 01 00`

22 byte long (0x16) status packet payload:

Byte 4 Always `0`

Byte 5 Always `0D`

Byte 6 Always `0`

Byte 7 Always `2`

Byte 8 Power:  

- `00` Off
- `01` On 

Byte 9 Fan mode:

- `00` Manual
- `01` Sleep
- `02` Auto 

Byte 10 Manual Fan Speed Selected

- `00` (Occurs at startup for 1 packet)
- `01` Low
- `02` Med 
- `03` High

Byte 11 Screen Brightness

- `00` Screen Off
- `64` Screen Full (Screen illuminates briefly when another button is tapped while screen is off)

Byte 12 Screen

- `00` Off
- `01` On 

Byte 13 Current Fan Speed

- `00` Sleep
- `01` Low
- `02` Med
- `03` High
- `255` Power Off

Byte 14 Always `0`

Byte 15 PM2.5

Byte 16 PM2.5

Byte 17 Always `0`

Byte 18 Display Lock:

- `00` Unlocked
- `01` Locked

Byte 19 Fan Auto Mode: (Only configurable by the app)

- `00` Default, speed based on air quality
- `01` Quiet, air quality but no max speed
- `02` Efficient, based on room size

Byte 20 & 21 Efficient Area: 

- `3B 01` 100 sq ft (App Minimum)  0x013B is 315

- `EC 04` 400 sq ft (App Maximum) 0x04EC is 1,260

  Linear scale, not sure what the units are.

Byte 22 Always `0`

##### `01 E6 A5`

##### `01 60 A2`

##### `01 E0 A5`

##### `01 29 A1`

##### `01 31 40`

##### `01 E2 A5`

##### `01 00 A0`

##### `01 05 A1`

##### `01 E4 A5`

##### `01 65 A2` - Timer Packet

MCU sends a packet when timer is running with remaining time

`A5 12 27 0C 00 DA 01 65 A2 00 08 0D 00 00 10 0E 00 00`

0x0D08 remaining seconds, Not sure on the rest.

#### Payload structure:



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
