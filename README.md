# RFID-Spool-Selector
**This is work in progress !!!!!!!!!!**

Hi! Here is my first project on GitHub. 
My 3D Printer stands in the garage and my Laptop is in the Diningroom inhouse. I use Octoprint on a RPi 4 1GB. To select the current filament in filamentmanager-plugin I must go inhouse, select the desired filament, go back and change filament. To optimize the worklow I bought a LCD/TFT-Display with touch from pollin.de and installed TouchUI. But this was to small and inperformant. So I planned a method/project to change the filament in filament-plugin directly.

# How to
**Hardware** 
- ESP32-Dev Board
- MFRC522 RFID Reader
- ST7735 SPI 1.8 TFT
- Rotary Encoder
- NTag-RFID Tags

**Software:**
 - [Octoprint](https://octoprint.org/) 
	 - [Filamentmanager-Plugin](https://plugins.octoprint.org/plugins/filamentmanager/)
 - MQTT-Broker
 - [Node-Red](https://nodered.org/)
 - [VS-Code](https://code.visualstudio.com/)/[Atom](https://atom.io/) with [Platformio](https://platformio.org/) or [Arduino](https://www.arduino.cc/)
	 - [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI)
	 - [MFRC522 library](https://github.com/miguelbalboa/rfid)
	 
Compile Sketch in Arduino or Platformio. 

## Pinout?
/*
 * -----------------------------------------------------------------------------------------
 *             MFRC522      ESP32         Display   Encoder
 *             Reader/PCD   
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          22             
 * RST                      14            RST
 * SPI SS      SDA(SS)      21            
 *                          17            CS
 * SPI MOSI    MOSI         23   
 * SPI MISO    MISO         19            MISO
 * SPI SCK     SCK          18            SCLK
 * DC                       4             DC    
 * VCC         VCC          3.3V          3.3V/BL   +
 * GND         GND          GND           GND       GND
 *                          5V            VCC
 *                          16                      SW
 *                          13                      DT
 *                          5                       CLK


<!--stackedit_data:
eyJoaXN0b3J5IjpbLTQ1NTc0OTYxLDI3NDkwNzYzNl19
-->
