#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS.h>
#include <Adafruit_HMC5883_U.h>
SoftwareSerial GPS(7,8); //GPS 
char outBuffer[40];
float drive, heading, Theading;
const float degtorad = 71 / 4068 , declination = 1.0 * degtorad , initp = 1013.5 ; //initial pressure and declination, needs adjusting every time starting
long lon , lat , Tlon, Tlat; //longtitude , lattitude , Target longtitude , Target lattitude
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
   String h = String(bme.readHumidity());
   Serial.println(t+";"+p+";"+h+";"+h);
   */
  //Formats all data onto a buffer that is printed in string form in the next line
  sprintf(outBuffer,"%s ; %s ; %s ; %s ; %s ",String(bme.readTemperature()),String(bme.readPressure()),String(bme.readAltitude(1013.25)),String(bme.readHumidity()),String(drive));
  Serial.println(String(outBuffer));
	
	
}

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));
  pinMode(2,INPUT);
	//Attaches interrupt to the pin recieving the pps signal
  attachInterrupt(digitalPinToInterrupt(3), Tick , RISING);
  if (!bme.begin()) {  
    Serial.println(F("NO BME"));
  }
  if(!mag.begin()){
    Serial.println(F("NO HMC"));
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
  //Makes a sensor event and reads all sensor data to it
  sensors_event_t event;
  mag.getEvent(&event); 
  //Reads calculates the direction we are heading
  heading = (atan2(event.magnetic.y, event.magnetic.x) + declination)/degtorad;
  if(heading < 0){ //Check if the value is negative
    heading += 360;
  } 
  else if(heading > 360){ // Check for wrap due to addition of declination.
    heading -= 360;
  }
  //Calculate Heading to target  N = 0 E = 90 S = 180 W = 270
  course = course_to(lat,lon,Tlat,Tlon);
  /*Calculating the drive variable in two steps: 
  Step one is to calculate the change in angle needed to make heading equal to the course.
  Step two is to calculate the (propotional) drive by dividing the step one number with the change of angle in degrees that we want the drive to be maximum and clamp the result between the maximal and the minimal drive value
  */
  drive = (course - heading) / 45
  drive = constrain(drive, -1 ,1);
  
	if(Serial.available() > 1){
    sIN = Serial.readString();
		switch(sIN.charAt(0)){
				case char(v){
				  Tlat = float(sIN.subString(1, sIN.indexOf(';')));
					Tlon = float(sIN.subString(sIN.indexOf(';'), sIN.length()));
	
				}
		}
	
   
}
