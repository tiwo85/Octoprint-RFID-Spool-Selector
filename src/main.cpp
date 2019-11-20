#include <Arduino.h>
#include <version.h>

/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

#include <WebServer.h>
#include <AutoConnect.h>

WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4

#define DEBUG
#define TFT_GREY 0x2104 // Dark grey 16 bit colour
#define SS_PIN    21
#define RST_PIN   22

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

const char* mqtt_server = "192.168.2.54";
const char* input1= "octoPrint/filamentManager/currentSpool";
const char* cmdtopic = "octoPrint/filamentManager/cmd";
int readID = 0;
int writeID = 0;
int isID = 0;
bool new_Data = true;
bool AutoUpdate = true; // true: Autoupdate Filament, depending on stored ID
bool writingnewID = false;
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
WiFiClient espClient;
PubSubClient client(espClient);

//JSON Defines
int id ; // 2
float cost ; // 15.46
const char* vendor ; // "Material4Print"
const char* material ; // "PLA"
const char* name ; // "Gelb"
float used ; // 27.7033317429915
int weight ; // 750
const char* command; // "selection"
bool issetup = true;
bool firststart = true;
byte left = 0;//left filament in percent
uint32_t runTime = -99999; 
byte oldID;


MFRC522::StatusCode status; //variable to get card status

  byte buffer[18];  //data transfer buffer (16+2 bytes data+CRC)
  byte oldRSSI;

  byte size = sizeof(buffer);

  uint8_t pageAddr = 0x06;  //In this example we will write/read 16 bytes (page 6,7,8 and 9).
                            //Ultraligth mem = 16 pages. 4 bytes per page. 
                            //Pages 0 to 4 are for special functions.  

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Config.autoReconnect = true;
  Portal.config(Config);
/*   Serial.print("Connecting to ");
  tft.print("Connecting to ");
  tft.println(ssid);
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print("."); 
    }*/
  if (Portal.begin()) {
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  tft.println("");
  tft.println("WiFi connected");
  tft.println("IP address: ");
  tft.println(WiFi.localIP());
  }
  


}


// #########################################################################
//  Rainbow Handle for Meter
// #########################################################################
unsigned int rainbow(byte value) {
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}



// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################

int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 4;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 0; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 6; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 10; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = TFT_RED; break; // Fixed colour
      case 1: colour = TFT_GREEN; break; // Fixed colour
      case 2: colour = TFT_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = TFT_BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREY);
    }
  }

  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);

  // Set the text colour to default
  //tft.setTextColor(TFT_WHITE,TFT_BLACK);
  /* tft.setTextColor(TFT_WHITE, TFT_BLACK); */
  // Uncomment next line to set the text colour to the last segment value!
   tft.setTextColor(text_colour, TFT_BLACK);
  
  // Print value, if the meter is large then use big font 6, othewise use 4
  if (r > 41) tft.drawCentreString(buf, x - 5, y - 20, 4); // Value in middle
  else tft.drawCentreString(buf, x - 5, y - 20, 2); // Value in middle

  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (r > 41) tft.drawCentreString(units, x, y + 30, 2); // Units display
  else tft.drawCentreString(units, x, y + 5, 1); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}


// #########################################################################
//  Calculate RSSI
// #########################################################################
byte rssi() {
  byte rssi_return;
 switch (WiFi.RSSI())
 {
 case -100 ... -90:
  rssi_return = 0;
  break;
 case -89 ... -80:
  rssi_return = 1;
  break;
 case -79 ... -70:
  rssi_return = 2;
  break;
 case -69 ... -60:
  rssi_return = 3;
  break;
 case -59 ... -50:
  rssi_return = 4;
  break;
 default:
  rssi_return = 5;
  break;
 }
 return rssi_return;
}

// #########################################################################
//  Draw Wifi-Signal-Symbol
// #########################################################################
void drawWifi(byte x,byte y,uint32_t foreground,uint32_t background,byte cubes){ 
  byte rssitest = map(WiFi.RSSI(),-100,-50,0,cubes);

  if(cubes==5){
    tft.fillRect(x,y+14,3,3,foreground);
    tft.fillRect(x+4,y+12,3,5,foreground);
    tft.fillRect(x+8,y+9,3,8,foreground);
    tft.fillRect(x+12,y+5,3,12,foreground);
    tft.fillRect(x+16,y,3,17,foreground);
    tft.fillRect((x+20)-((5-rssitest)*4),y,((5-rssitest)*4), 18, background);
  }
  else
  {
    tft.fillRect(x,y+5,3,3,foreground);
    tft.fillRect(x+4,y+3,3,5,foreground);
    tft.fillRect(x+8,y,3,8,foreground);
    tft.fillRect((x+(cubes*4))-((cubes-rssitest)*4),y,((cubes-rssitest)*4), 8, background);
  }
  
}

// #########################################################################
//  MQTT Callback and Reconnect Handle
// #########################################################################
void callback(char* topic, byte* payload, unsigned int length) {
  
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
 const size_t capacity = JSON_OBJECT_SIZE(8) + 100;
 DynamicJsonDocument doc(capacity);

 deserializeJson(doc, payload,length);

 id = doc["id"]; // 2
 isID = id;
 cost = doc["cost"]; // 15.46
 vendor = doc["vendor"]; // "Material4Print"
 material = doc["material"]; // "PLA"
 name = doc["name"]; // "Gelb"
 used = doc["used"]; // 27.7033317429915
 weight = doc["weight"]; // 750
 command = doc["command"]; // "selection"
 #ifdef DEBUG
 Serial.print("id: ");
 Serial.println(id);
 Serial.print("cost: ");
 Serial.println(cost);
 Serial.print("vendor: ");
 Serial.println(vendor);
 Serial.print("material: ");
 Serial.println(material);
 Serial.print("name: ");
 Serial.println(name);
 Serial.print("used: ");
 Serial.println(used);
 Serial.print("weight: ");
 Serial.println(weight);
 Serial.print("command: ");
 Serial.println(command);
 left = 100-(used * 100 / weight);

 #endif
 if(readID==0){
   readID = isID;
 }
 if(oldID!=isID){
 new_Data = true;
 oldID = isID;
 }
  
} 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if(issetup) tft.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      if(issetup) tft.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe("octoPrint/filamentManager/currentSpool");            
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      if(issetup) tft.print("failed, rc=");
      if(issetup) tft.println(client.state());
      Serial.println(" try again in 5 seconds");
      if(issetup) tft.println("try in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}
// #########################################################################
//  Draw Screen
// #########################################################################
void displayScreen1(){
  
  //MQTT Status
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_DARKGREY,TFT_BLACK);  
  tft.setCursor(28,1)  ;
  if(!writingnewID){
    if(AutoUpdate){
      
      tft.print(" Autoupdate ");
    }
    else {
      tft.print("   Manual   ");
    }
  }
  else {
    tft.print("Writing Mode");
  }
  //-----------Filament Info:
  //tft.drawRoundRect(0,20,128,52,4,TFT_WHITE);
  tft.fillRect(0,20,128,140,TFT_BLACK);
  tft.setTextFont(2);
  
  if(isID==readID) {
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
  }
  else {
    tft.setTextColor(TFT_YELLOW,TFT_BLACK);
  }
  tft.setCursor(3, 14);
  char bufferdata[50];
  sprintf(bufferdata,"ID: %3d ",isID);
  tft.println(bufferdata); 
  tft.drawRightString(String(material),126,14,2);
  tft.setTextDatum(0);
  tft.setCursor(2,30);
  tft.println(vendor);
  tft.setCursor(2,46);
  tft.println(name);
  // Rechteck
  tft.drawRoundRect(0,12,128,52,4,TFT_WHITE);
  tft.drawRoundRect(1,13,126,50,4,TFT_BLACK);
  int  ypos = 72, gap = 4, radius = 44;
  int xpos = (128 - (radius*2))/2; //center ringMeter
  xpos = gap + ringMeter(left, 0, 100, xpos, ypos, radius, "%", RED2GREEN); 
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(2);
  tft.setCursor(0,72);
  tft.print((int)used);
  tft.setTextFont(1);
  tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  tft.setCursor(0,87);
  tft.print("used");
  tft.setCursor(0,140);
  tft.print("left");
  tft.setCursor(104,140);
  tft.print("full");
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(2,147);
  int rest = (int)weight - (int)used;
  tft.print(rest);
  tft.drawRightString(String((int)weight),127,147,2);
 //drawWifi(109,143);

  new_Data = false;
}

// #########################################################################
//  Setup
// #########################################################################
void setup() {
	Serial.begin(115200);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
    tft.init();
  //tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_BLUE);    tft.setTextFont(4);
  tft.println("Boot");
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(1);
  tft.println(VERSION_SHORT);
  tft.print("Build: ");
  tft.println(BUILD_NUMBER);
  Serial.println();
  Serial.println();
  Serial.println(VERSION);
  Serial.print("Build: ");
  Serial.println(BUILD_NUMBER);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();

	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
    tft.print("MFRC522 Ver: 0x");
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  tft.println(readReg, HEX);
  Serial.println(F("Sketch has been started!"));

  memcpy(buffer,"nID=006",7);
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  new_Data = true;
  issetup = false;
}
// #########################################################################
//  Writing Handle for NTag
// #########################################################################
void writeNTag(){
  // Look for new cards

  // Write data ***********************************************
 memcpy(buffer,"nID=006",7);

    for (int i=0; i < 4; i++) {
    //data is writen in blocks of 4 bytes (4 bytes per page)
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Ultralight_Write(pageAddr+i, &buffer[i*4], 4);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
  }
  Serial.println(F("MIFARE_Ultralight_Write() OK "));
  Serial.println();
  writingnewID = false;
}
// #########################################################################
//  Change Spool
// #########################################################################
void changeSpool(){
  char publishbuffer[8] ;
  sprintf(publishbuffer,"\"id\":%d",readID);
  client.publish(cmdtopic,(char *)publishbuffer);
}
// #########################################################################
//  Read NTag
// #########################################################################
void readNTag(){



    // Read data ***************************************************
  Serial.println(F("Reading data ... "));
  //data in 4 block is readed at once.
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(pageAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  Serial.print(F("Readed data: "));
  //Dump a byte array to Serial
  for (byte i = 1; i < 7; i++) {
    Serial.write(buffer[i]);
  }
   Serial.println();
   byte buffer2[3];
  for (int a = 0; a < 3; a++) {
  Serial.write(buffer[a+4]);
  buffer2[a]=buffer[a+4];
  }
  Serial.println();
 /*  String myString = (char*)buffer;
  Serial.println();
  Serial.println(myString); */
  Serial.print("AlteID:");
  Serial.println(readID);  
  int bufferID =  (buffer2[2]-48) ;
  bufferID +=  (buffer2[1]-48)*10 ;
  bufferID +=  (buffer2[0]-48)*100 ;
  Serial.print("NTag ID:");
  Serial.println(bufferID);
  if(bufferID == readID){
    Serial.println("Alles beim Alten");
    return;
  }
  readID = bufferID;
  Serial.print("Neue ID:");
  Serial.println(readID);
  Serial.println();
  
 new_Data = true;
  
}

// #########################################################################
//  Handle NTag
// #########################################################################
void computeNTag(){
    if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  if(writingnewID==true){
    writeNTag();
  }

  readNTag();

  if(readID > 255 or readID < -255){
    writeNTag();
    readNTag();
  }

   if(AutoUpdate==true) {
     changeSpool();
   }
 mfrc522.PICC_HaltA();
}

// #########################################################################
//  Loop
// #########################################################################
void loop() {
  if (!client.connected()) {
  /*     tft.fillCircle(5,154,2,TFT_RED);
    new_data =true; */
    reconnect();
  }
    Portal.handleClient();
 if (millis() - runTime >= 2000L) { // Execute every 2s
  runTime = millis(); 
  if(rssi()!=oldRSSI) 
  {
    drawWifi(115,0,TFT_DARKGREY,TFT_BLACK,3);
    oldRSSI = rssi();
  }
 }
  computeNTag();

  client.loop();

  if (new_Data == true) displayScreen1();
 
}
