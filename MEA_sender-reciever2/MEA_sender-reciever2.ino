
/***********************************
 *  Safety MEA(n) U Project
 *  Created by Supavadee Phusanam
************************************/

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Libraries for accelerometer
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
Adafruit_MPU6050 mpu;

//Blynk Setting
#define BLYNK_TEMPLATE_ID "TMPLJPAzvIg7"
#define BLYNK_DEVICE_NAME "SafetyMeanU Sender"
#define BLYNK_AUTH_TOKEN "5rZpKgGKlObLP4wxeZkMpP9M5FJsI9pd"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

//Library for Blynk
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Config
char auth[] = BLYNK_AUTH_TOKEN;       // Auth Token provied by Blynk app
const char ssid[] = "yingspvd";       //  Wifi name
const char password[] = "88888888";   //  wifi password 

BlynkTimer timer;

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

/*LoRa send Data*/
String outgoing;              // outgoing message
int ID = 4863;                // count of outgoing messages
String location = "https://www.google.com/maps?q=%E0%B8%AD%E0%B8%A1%E0%B8%B2%E0%B8%A3%E0%B8%B5+%E0%B8%9B%E0%B8%A3%E0%B8%B0%E0%B8%95%E0%B8%B9%E0%B8%99%E0%B9%89%E0%B8%B3&um=1&ie=UTF-8&sa=X&ved=2ahUKEwjO0_Cnrpv2AhUi73MBHVuVB7QQ_AUoAXoECAIQAw";
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 00000002;     // address of this device 
byte destination = 00000004;      // destination to send to

double acx = 0;
double acy = 0;
double acz = 0;
double ac = 0;
double ac_max=0;
double force = 0;
double force_max = 0;
double mass = 1670;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  resetValue();
  
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
  display.print("LORA SENDER ");
  display.display();
  
  Serial.println("LoRa Sender Test");

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
  display.print("LoRa Initializing OK!");
  display.display();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");

  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  Serial.print("Filter bandwidth set to: ");
  Serial.println("");
  delay(100);
  
  delay(2000);
}

void loop() {
  Blynk.run();
   /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  acx = a.acceleration.x;
  acy = a.acceleration.y;
  acz = a.acceleration.z;

  ac = sqrt(pow(acx,2) + pow(acy,2) + pow(acz,2));
  force = mass * ac;

  if(ac > ac_max){
    ac_max = ac;
  }

  if(force > force_max){
    force_max = force;
  }
  
  Blynk.virtualWrite(V1,ID);
  Blynk.virtualWrite(V2,ac);
  Blynk.virtualWrite(V3,ac_max);  
  Blynk.virtualWrite(V4,force); 
  Blynk.virtualWrite(V5,force_max); 
  
  display.clearDisplay();
  display.setCursor(0,0);

  /*If Crash*/
  if(ac >= 25){
    sendMessage(2);
  }
  else if(ac >= 15 && ac < 25){
    sendMessage(1);
  }
  else{
    display.print("Not Crash");
  }
  display.display();

   // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}

void sendMessage(int level){
    String outgoing = "";         // send a message
    outgoing = String(ID) + ">" + location + "*" + String(ac_max) + "<" + String(force_max)+ "@" + String(level);
    LoRa.beginPacket();                   // start packet
    LoRa.write(destination);              // add destination address
    LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);                 // add message ID
    LoRa.write(outgoing.length());        // add payload length
    LoRa.print(outgoing);                 // add payload
    LoRa.endPacket();                     // finish packet and send it
    msgCount++;                           // increment message ID
    
    Serial.print("Electric Pole ID: ");
    Serial.println(ID);
    display.print("Crash");
    delay(5000);
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  Serial.print("packetSize");
  Serial.println(packetSize);
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 00000002) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  resetValue();
}

void resetValue(){
  ac = 0;
  ac_max = 0;
  force = 0;
  force_max = 0;
  Blynk.virtualWrite(V2,0);
  Blynk.virtualWrite(V3,0);
  Blynk.virtualWrite(V4,0);
  Blynk.virtualWrite(V5,0);
}
