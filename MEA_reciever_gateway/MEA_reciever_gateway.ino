
/***********************************
 *  Safety MEA(n) U Project
 *  Created by Supavadee Phusanam
************************************/
#include <TridentTD_LineNotify.h>

#include <ssl_client.h>
#include <WiFiClientSecure.h>

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Blynk Setting
#define BLYNK_TEMPLATE_ID "TMPLq1msm7AF"
#define BLYNK_DEVICE_NAME "SafetyMeanU Receiver"
#define BLYNK_AUTH_TOKEN "Pn_5LY8yoSXUUSt_lnbB6odapvRW9VWE"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

//Library for Blynk
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Blynk token
char auth[] = BLYNK_AUTH_TOKEN;     // Auth Token provied by Blynk app

// WIFI
const char SSID[] = "yingspvd";     // Wifi name 
const char PASSWORD[] = "88888888"; // Wifi password 

#define LINE_TOKEN  "oqcae9Hg3ZAryK8x6nPmGozyxSqSM9GVucwrfVJmouW"

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 915E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Data
String LoRaData;
String location;
String ID;
String ac;
String force;
String level;

void setup() { 
  //initialize Serial Monitor
  Serial.begin(115200);
  Blynk.begin(auth, SSID, PASSWORD);
  Serial.println(LINE.getVersion());

  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while(WiFi.status() != WL_CONNECTED){ 
    Serial.print("."); 
    delay(400); 
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  

  LINE.setToken(LINE_TOKEN);
  LINE.notifyPicture("https://photos.app.goo.gl/nkW5CN8Ld67oeutRA");
  LINE.notifySticker("Let's Start",11538,51626494);

  
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER ");
  display.display();
  
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.println("LoRa Initializing OK!");
  display.display();  
}

void loop() {
  Blynk.run();
  readData();
  displayData();
}

void readData(){
  byte localAddress = 00000004;     // address of this device
  int count = 0;
  
  //try to parse packet
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    //received a packet
    Serial.print("Received packet ");
    int recipient = LoRa.read();          // recipient address
    byte sender = LoRa.read();            // sender address
    byte incomingMsgId = LoRa.read();     // incoming msg ID
    byte incomingLength = LoRa.read();    // incoming msg length
    String incoming = "";

    //read packet
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
Serial.println(incomingLength);
Serial.println(incoming.length());
    if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
    }

    // if the recipient isn't this device or broadcast,
    if (recipient != localAddress && recipient != 00000004) {
      Serial.println("This message is not for me.");
      return;                             // skip rest of function
    }
    
    // if message is for this device, or broadcast, print details:
    int pos0 = incoming.indexOf('>');
    int pos1 = incoming.indexOf('*');
    int pos2 = incoming.indexOf('<');
    int pos3 = incoming.indexOf('@');

    ID = incoming.substring(0, pos0);
    location = incoming.substring(pos0+1, pos1);
    ac = incoming.substring(pos1+1, pos2);
    force = incoming.substring(pos2+1, pos3);
    level = incoming.substring(pos3+1, incoming.length());

    String message = "ID: " + ID;
    String messageLine =  "\nID: " + ID  + "\nLevel: " + level + "\nLocation: " + location;
    Serial.println("Received from: 0x" + String(sender, HEX));
    Serial.println("Sent to: 0x" + String(recipient, HEX));
    Serial.println("ID: " + ID);   
    Serial.println("Location: " + location);   
    Serial.println("Accelration: "+ac);   
    Serial.println("Force: " + force); 
    Serial.println("Level: " + level); 
    Blynk.logEvent("level2"+ message);
    LINE.notify(messageLine);
    
    Blynk.virtualWrite(V0,ID);
    Blynk.virtualWrite(V1,ac);  
    Blynk.virtualWrite(V2,force);  
    Blynk.virtualWrite(V3,level);  
      
    }
}

void displayData(){
  // Display information
   display.clearDisplay();
   display.setCursor(0,0);
   display.print("LORA RECEIVER");
   display.setCursor(0,20);
   display.print("Received packet:");
   display.setCursor(0,30);
   display.print(LoRaData);
   display.display();   
}

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V4)
{
  resetValue();
}

void resetValue(){
  ID = "";
  ac = "";
  force = "";
  level = "";
  Blynk.virtualWrite(V0,0);
  Blynk.virtualWrite(V1,0);
  Blynk.virtualWrite(V2,0);
  Blynk.virtualWrite(V3,0);
  
}
