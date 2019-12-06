/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMEP280 Breakout 
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required 
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS.h>
#include <Adafruit_HMC5883_U.h>
SoftwareSerial GPS(7,8); //GPS
TinyGPS gps;
char outBuffer[40];
int initp = 1013.5;
long lon , lat;
unsigned long fix_age;
float drive = 0;
/*
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 9
#define BMP_CS 8
*/
Adafruit_BME280 bme; // I2C
Adafruit_HMC5883_Unified mag;

//Stuurt informatie en stuurt motoren iedere seconde 
void Tick(){
  /*
   String t = String(bme.readTemperature()); 
   String p = String(bme.readPressure()); 
   String h = String(bme.readAltitude(1013.25)); 
   String m = String(bme.readMoisture());
   Serial.println(t+";"+p+";"+h+";"+m);
   */
  sprintf(outBuffer,"%f ; %f ; %f ; %f",String(bme.readTemperature()),String(bme.readPressure()),String(bme.readAltitude(1013.25)),String(bme.readHumidity()));
  Serial.println(String(outBuffer));
}

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));
  pinMode(2,INPUT);
  attachInterrupt(digitalPinToInterrupt(2), Tick , RISING);
  if (!bme.begin()) {  
    Serial.println(F("NO BME"));
  }
  if(!mag.begin()){
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("NO HMC");
    while(1);
  }

}

void loop() {
  while (GPS.available())
  {
    if (gps.encode(GPS.read()))
    {
      gps.get_position(&lat, &lon, &fix_age);
    }
  }
  sensors_event_t event;
  mag.getEvent(&event); 
}
