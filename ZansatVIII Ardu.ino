#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS.h>  
#include <Adafruit_HMC5883_U.h>

//char Buffer[40]; 
String out; 
float initp = 1013.25 , declination = 1.0 * degtorad /*default*/,drive, heading, course;
const float degtorad = 71 / 4068 ,  ; //initial pressure and declination, needs adjusting every time starting
long lon , lat , Tlon, Tlat; //longtitude , lattitude , Target longtitude , Target lattitude
unsigned long fix_age;
bool servoEN = False; //Servo motors responsible for guidance are disabled at the start and can be enabled by command

#define GPSIN  8
#define GPSOUT 7
#define GPSPPS 3

SoftwareSerial GPS(GPSIN,GPSOUT); //GPS 
TinyGPS gps;
Adafruit_BME280 bme; // I2C
Adafruit_HMC5883_Unified mag;

//Sends information every interrupt
void Tick(){
   out = out + String(bme.readTemperature()); 
   out = out + String(bme.readPressure()); 
   out = out + String(bme.readAltitude(initp)); 
   out = out + String(bme.readHumidity());
   Serial.println(out);
   out = ''
  //Formats all data onto a buffer that is printed in string form in the next line
  //sprintf(outBuffer,"%s ; %s ; %s ; %s ; %s ",String(bme.readTemperature()),String(bme.readPressure()),String(bme.readAltitude(1013.25)),String(bme.readHumidity()),String(drive));
  //Serial.println(String(outBuffer));
	
	
}

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
  pinMode(2,INPUT);
	//Attaches interrupt to the pin recieving the pps signal
  attachInterrupt(digitalPinToInterrupt(GPSPPS), Tick , RISING);
  if (!bme.begin()) {  
    Serial.println(F("NO BME"));
    while(1);
  }
  if(!mag.begin()){
    Serial.println(F("NO HMC"));
    while(1);
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
  //Reads calculates the direction we are heading. TODO: Check Sensor orientation and change formula accordingly 
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
  //Recieving commands from the ground
	if(Serial.available() > 1){
    sIN = Serial.readString();
		switch(sIN.charAt(0)){ //Commands: C ARG1 ARG2 ARG3 Example: s 1 
      case char('v'): //Setting Victim Coordinates 
				  Tlat = sIN.subString(2, sIN.indexOf(' ')).toFloat();
					Tlon = sIN.subString(sIN.indexOf(' ')).toFloat();
          break;
      case char('i'): //Setting initial pressure and declination
          initp = sIN.subString(2, sIN.indexOf(' ')).toFloat();
					declination = sIN.subString(sIN.indexOf(' ')).toFloat() * degtorad;
          break;
      case char('s'): //Enabling / Disabling servos (switch if not argumented , on if arg is 'ON' , off if anything else, Argument of max 2 chars) 
          if (char-1){
            servoEN = sIN.substring(2,4) == 'ON'
          } 
          else {
            servoEN = !servoEN;
          }
        
          
          
		}
	
   
}
