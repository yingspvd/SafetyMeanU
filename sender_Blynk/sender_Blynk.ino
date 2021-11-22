
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

char auth[] = BLYNK_AUTH_TOKEN; // Enter the Auth Token provied by Blynk app
//const char ssid[] = "somboon_5G-pro-2.4G"; // Enter your Wifi name 
const char ssid[] = "Ying's iPhone"; // Enter your Wifi name +
const char password[] = "88888888"; // Enter wifi password 

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
byte ID = 4;                  // count of outgoing messages
byte localAddress = 0xBB;     // address of this device 
byte destination = 0xFF;      // destination to send to

// MAX value
double x_max = -100000;
double y_max = -100000;
double z_max = -100000;


// MIN value
double x_min = 10000;
double y_min = 10000;
double z_min = 10000;

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

  /*Show acceleration */
  Serial.print(a.acceleration.x);
  Serial.print(' ');
  Serial.print(a.acceleration.y);
  Serial.print(' ');
  Serial.println(a.acceleration.z);

//  Set Max
  if(a.acceleration.x > x_max){
    x_max = a.acceleration.x;
  }
  if(a.acceleration.y > y_max){
    y_max = a.acceleration.y;
  }
  if(a.acceleration.z > z_max){
    z_max = a.acceleration.z;
  }
//  Set Min
  if(a.acceleration.x < x_min){
    x_min = a.acceleration.x;
  }
  if(a.acceleration.y < y_min){
    y_min = a.acceleration.y;
  }
  if(a.acceleration.z < z_min){
    z_min = a.acceleration.z;
  }
  Blynk.virtualWrite(V3,ID);
  Blynk.virtualWrite(V0, a.acceleration.x);
  Blynk.virtualWrite(V1, a.acceleration.y);
  Blynk.virtualWrite(V2, a.acceleration.z);
  Blynk.virtualWrite(V4, x_max);
  Blynk.virtualWrite(V5, y_max);
  Blynk.virtualWrite(V6, z_max);
  Blynk.virtualWrite(V7, x_min);
  Blynk.virtualWrite(V8, y_min);
  Blynk.virtualWrite(V9, z_min);  
  
  display.clearDisplay();
  display.setCursor(0,0);

  /*If Crash*/
 // a.acceleration.y > 30
  if(a.acceleration.y > 20){
    sendMessage();
  }
  else{
    display.print("Not Crash");
  }
  display.display();

}

void sendMessage(){
    String message = "MEA";         // send a message
    LoRa.beginPacket();             // start packet
    LoRa.write(destination);        // add destination address
    LoRa.write(localAddress);       // add sender address
    LoRa.write(ID);                 // ID
    LoRa.write(message.length());   // add payload length
    LoRa.print(message);            // add payload
    LoRa.endPacket();
    
    Serial.print("Electric Pole ID: ");
    Serial.println(ID);
    display.print("Crash");
    delay(5000);
}

// This function is called every time the Virtual Pin 10 state changes
BLYNK_WRITE(V10)
{
  resetValue();
}

void resetValue(){
  double default_value = -5000;
  double min_value = 10000;
  double max_value = -10000;
  x_max = max_value;
  y_max = max_value;
  z_max = max_value;
  x_min = min_value;
  y_min = min_value;
  z_min = min_value;
  Blynk.virtualWrite(V3,0);
  Blynk.virtualWrite(V0, default_value);
  Blynk.virtualWrite(V1, default_value);
  Blynk.virtualWrite(V2, default_value);
  Blynk.virtualWrite(V4, max_value);
  Blynk.virtualWrite(V5, max_value);
  Blynk.virtualWrite(V6, max_value);
  Blynk.virtualWrite(V7, min_value);
  Blynk.virtualWrite(V8, min_value);
  Blynk.virtualWrite(V9, min_value);
}
