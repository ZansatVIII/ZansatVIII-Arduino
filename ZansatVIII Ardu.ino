#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS.h>
#include <Adafruit_HMC5883_U.h>
SoftwareSerial GPS(7,8); //GPS
char outBuffer[40];
float drive, heading, const declination = (1.0/180) * PI , const initp = 1013.5 ; //initial pressure, needs adjusting every time starting
long lon , lat; //longtitude , lattitude
unsigned long fix_age;
/*
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 9
#define BMP_CS 8
*/
TinyGPS gps;
Adafruit_BME280 bme; // I2C
Adafruit_HMC5883_Unified mag;

//Sends information every interrupt
void Tick(){
  /*
	 //Alternative way: Use if the one below does not work
   String t = String(bme.readTemperature()); 
   String p = String(bme.readPressure()); 
   String h = String(bme.readAltitude(1013.25)); 
   String m = String(bme.readHumidity());
   Serial.println(t+";"+p+";"+h+";"+m);
   */
  sprintf(outBuffer,"%f ; %f ; %f ; %f ; %Lf",String(bme.readTemperature()),String(bme.readPressure()),String(bme.readAltitude(1013.25)),String(bme.readHumidity()));
  Serial.println(String(outBuffer));
	
	
}

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));
  pinMode(2,INPUT);
	//Attaches interrupt to the pin recieving the pps signal
  attachInterrupt(digitalPinToInterrupt(2), Tick , RISING);
  if (!bme.begin()) {  
    Serial.println(F("NO BME"))
  }
  if(!mag.begin()){
    Serial.println("NO HMC");
  }

}

void loop() {
	//Gets Longtitude And Lattitude from the gps
  while (GPS.available())
  {
    if (gps.encode(GPS.read()))
    {
			gps.get_position(&lat, &lon, &fix_age);
			
    }
  }
	//Calculates the direction we are heading
	sensors_event_t event;
  mag.getEvent(&event); 
	heading = atan2(event.magnetic.y, event.magnetic.x) + declination
	if(heading < 0){ //Check if the value is negative
    heading += 2*PI;
	} 
	else if(heading > 2*PI){ // Check for wrap due to addition of declination.
	  heading -= 2*PI;
	}
    
	
  
}
