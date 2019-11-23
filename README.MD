<h1 id="rfid-spool-selector">RFID-Spool-Selector</h1>
<p><strong>This is work in progress !!!</strong></p>
<p>Hi! Here is my first project on GitHub.<br>
My 3D Printer stands in the garage and my Laptop is in the Diningroom inhouse. I use Octoprint on a RPi 4 1GB. To select the current filament in filamentmanager-plugin I must go inhouse, select the desired filament, go back and change filament. To optimize the worklow I bought a LCD/TFT-Display with touch from <a href="http://pollin.de">pollin.de</a> and installed TouchUI. But this was to small and inperformant. So I planned a method/project to change the filament in filament-plugin directly.</p>
<h1 id="how-to">How to</h1>
<p><strong>Hardware</strong></p>
<ul>
<li>ESP32-Dev Board</li>
<li>MFRC522 RFID Reader</li>
<li>ST7735 SPI 1.8 TFT</li>
<li>Rotary Encoder</li>
<li>NTag-RFID Tags</li>
</ul>
<p><strong>Software:</strong></p>
<ul>
<li><a href="https://octoprint.org/">Octoprint</a>
<ul>
<li><a href="https://plugins.octoprint.org/plugins/filamentmanager/">Filamentmanager-Plugin</a></li>
</ul>
</li>
<li>MQTT-Broker</li>
<li><a href="https://nodered.org/">Node-Red</a></li>
<li><a href="https://code.visualstudio.com/">VS-Code</a>/<a href="https://atom.io/">Atom</a> with <a href="https://platformio.org/">Platformio</a> or <a href="https://www.arduino.cc/">Arduino</a>
<ul>
<li><a href="https://github.com/Bodmer/TFT_eSPI">TFT_eSPI library</a></li>
<li><a href="https://github.com/miguelbalboa/rfid">MFRC522 library</a></li>
</ul>
</li>
</ul>
<p>Compile Sketch in Arduino or Platformio.</p>
<h2 id="pinout">Pinout?</h2><pre>
<code>
  -           MFRC522      ESP32         Display   Encoder
  -           Reader/PCD   
  Signal      Pin          Pin           Pin       Pin     
  ---------------------------------------------------------
  RST/Reset   RST          22
  RST                      14            RST
  SPI SS      SDA(SS)      21
  CS          -            17            CS
  SPI MOSI    MOSI         23
  SPI MISO    MISO         19            MISO
  SPI SCK     SCK          18            SCLK
  DC          -            4             DC    
  VCC         VCC          3.3V          3.3V/BL   +
  GND         GND          GND           GND       GND
  5V          -            5V            VCC
  SW          -            16                      SW
  DT          -            13                      DT
  CLK         -            5                       CLK
 </code>
</pre>
