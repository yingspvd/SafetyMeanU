
/***********************************
 *  Safety MEA(n) U Project
************************************/

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
const char ssid[] = "yingspvd";     // Wifi name 
const char password[] = "88888888"; // Wifi password 

//Library for Line Notify
#include <TridentTD_LineNotify.h>
#define LINE_TOKEN "NVz2bXWDzMGfDt2JNCz0DipsBnVbv2iGLrz3jHy3AwJ"

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
String ID;
String ac;
String force;
String level;

void setup() { 
  //initialize Serial Monitor
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  
  Serial.println(WiFi.localIP());
  Serial.println(LINE.getVersion());

  Serial.print("LINE_TOKEN");
  Serial.println(LINE_TOKEN);
  LINE.setToken(LINE_TOKEN);
  
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
  int count = 0;
  //try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //received a packet
    Serial.print("Received packet ");
      
    //read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.println(LoRaData);

      int pos1 = LoRaData.indexOf('/');
      int pos2 = LoRaData.indexOf('&');
      int pos3 = LoRaData.indexOf('-');

      ID = LoRaData.substring(0, pos1);
      ac = LoRaData.substring(pos1+1, pos2);
      force = LoRaData.substring(pos2+1, pos3);
      level = LoRaData.substring(pos3+1, LoRaData.length());

      String message = "ID: " + ID;
      Serial.println(ID);   
      Serial.println(ac);   
      Serial.println(force); 
      Serial.println(level); 
      Blynk.logEvent("level1",message);

      Blynk.virtualWrite(V0,ID);
      Blynk.virtualWrite(V1,ac);  
      Blynk.virtualWrite(V2,force);  
      Blynk.virtualWrite(V3,level);  
      }
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
