# BMW Ibus for Teensy 4.1
Used and modified Library is a fork from [GitHub Pages] https://github.com/just-oblivious/arduino-ibustrx

## Ibus Aufbau:
example IBUS message:
50 04 68 32 11 1F (volume up button pressed on the steering wheel)
|  |  |  |  |  | 
|  |  |  |  |  checksum (xorsum of all previous bytes)
|  |  |  |  one or more data fields
|  |  |  message type/command type
|  |  destination address
|  length of message (including destination address and checksum)
source address 

## Teensy 4 / 4.1:
ARM Cortex-M7 at 600 MHz
1024K RAM (512K is tightly coupled)
8192K Flash (64K reserved for recovery & EEPROM emulation)
2 USB ports, both 480 MBit/sec
1 x Ethernet PHY (10/100 Mbit DP83825)
3 CAN Bus (1 with CAN FD)
2 I2S Digital Audio
1 S/PDIF Digital Audio
1 SDIO (4 bit) native SD
3 SPI, all with 16 word FIFO
3 I2C, all with 4 byte FIFO
7 Serial, all with 4 byte FIFO
32 general purpose DMA channels
35 PWM pins
55 I/O pins:
42 digital pins, all interrrupt capable, breadboard friendly
6 pins in SD Card slot
7 pins on SMT pads
18 analog pins, 2 ADCs on chip
Cryptographic Acceleration
Random Number Generator
RTC for date/time
Programmable FlexIO
Pixel Processing Pipeline
Peripheral cross triggering

## PINOUT
- LED Buildin = 13
- für evtl CAN =  Can1     = RxPin 23 und TxPin 22
- für Ibus =      Serial2  = RxPIN 7 und TxPin 8 , CleartoSent = Pin 5
- für GSM/GPS =   Serial1  = RxPin 0 , TxPin 1 , Powerbutton 3 , DTR 2
- Licht Sensor BH1750 über I²C = SCL pin19, SDA pin 18, Vcc=3.3V


## Was bis jezt implementiert ist:
- Tipplinken mit IKE-Dimmer-Anpassung
- Schlüsselerkennung und Text-Begrüßung im IKE
- Heimleuchten mit Lichtsensor BH1750
- ZV-Verrieglung bei Speed > 30 und Entriegelung bei Motor aus
- Geschwindigkeit und Außentemperatur
- Kühlmitteltemperatur wird im Boadcomputer angezeigt anstatt Timer 2



## ToDo:
- Standheizung via ibus Ein/Aus
- Alle Bordcomputerdaten auslesen - noch keinen ibus code gefunden!
- Tankanzeige auslesen - noch keinen ibus code gefunden!

## Erstes PCB Layout zum weiteren austesten:
![Alt text](/Pics/TOP_Side_02.png )