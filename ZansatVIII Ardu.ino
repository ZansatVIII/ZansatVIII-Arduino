//////////////////////////////////////////////////////////Info\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
/*                                       Zansat VIII cansat microcontroller code

                              Github Source: https://github.com/ZansatVIII/ZansatVIII-Arduino/
                          Author: Jakub Stachurski, Lead programmer @ the Zansat VIII Cansat Team 
                      
                        !! Disclaimer: This code is designed for a specific board and components, 
                        different hardware configurations may not result in optimal performance. !!
                       
              This version is not fully tested nor optimized, take this into account when using the code in any way: 
                                        The only test made is compiling this code 
                                           
                                          The code is licensed under GGPL v3.0:
                               (TL;DR : Plagiarizing this is fine as long as you admit to it)
                                    
                                        ... An output parsing tool coming soon ...
*/
///////////////////////////////////////////////////////////Libs\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS.h>  
#include <Adafruit_HMC5883_U.h>
#include <SD.h>
#include <Servo.h>
///////////////////////////////////////////////////////Defines and vars\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

String out; 
const float degtorad = 71 / 4068; //initial pressure and declination, needs adjusting every time starting
float initp = 1013.25 , declination = 1.0 * degtorad /*default*/,drive, heading, course;
long lon , lat , Tlon, Tlat; //longtitude , lattitude , Target longtitude , Target lattitude
unsigned long fix_age;
bool servoEN = false; //Servo motors responsible for guidance are disabled at the start and can be enabled by command
File Log ;
Servo SL , SR;

#define GPSIN  8
#define GPSOUT 7
#define GPSPPS 3
#define SDSS 13
#define SRVL 5
#define SRVR 6

SoftwareSerial GPS(GPSIN,GPSOUT); //GPS 
TinyGPS gps;
Adafruit_BME280 bme; // I2C
Adafruit_HMC5883_Unified mag;
///////////////////////////////////////////////////Sends all data every interrupt/////////////////////////////
//Adds the float as string onto the out string 

void INIT(){ 
  if(SD.begin(SDSS)){
    File Log = SD.open("log.txt", FILE_WRITE);
    if(Log){
      out = "START";
      }
    else {
      out = "SO";
    }
    SL.attach(SRVL);
    SR.attach(SRVR);
  }
}

void Append(float f){
  out = out + String(f) + ';' ;
}

void Tick(){
   Append(bme.readTemperature()); 
   Append(bme.readPressure()); 
   Append(bme.readAltitude(initp)); 
   Append(bme.readHumidity());
   Append(course);
   Append(drive);
   Serial.print(out);
   if(Log) {
    Log.print(out);
  }
   out = '\n';
  //Formats all data onto a buffer that is printed in string form in the next line
  //sprintf(outBuffer,"%s ; %s ; %s ; %s ; %s ",String(bme.readTemperature()),String(bme.readPressure()),String(bme.readAltitude(1013.25)),String(bme.readHumidity()),String(drive));
  //Serial.println(String(outBuffer));
}
////////////////////////////////////////////Hardware and Library initialization\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
  pinMode(2,INPUT);
  //Initializes all logging and measurement capabilites
  INIT();
  if (!bme.begin()) {  
    out = out + "B";
    while(1);
  }
  else{
    initp = bme.readPressure();
  }
  if(!mag.begin()){
    out = out + "M";
    while(1);
  }
  
  //Attaches interrupt to the pin recieving the pps signal
  attachInterrupt(digitalPinToInterrupt(GPSPPS), Tick , RISING);
}

void loop() {
  ///////////////////////////BEGIN OF LOOP | Orientation data gathering \\\\\\\\\\\\\\\\\\\\\\\
  //Gets Longtitude And Lattitude from the gps
  while (GPS.available())
  {
    if (gps.encode(GPS.read()))
    {
    gps.get_position(&lat, &lon, &fix_age);
      
    }
  }
  //Makes a sensor event and reads all magnetometer data to it
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
//////////////////////////////////////Drive Calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\  
  //Calculate Course to target  N = 0 E = 90 S = 180 W = 270 , Own heading must obey the same rules
  course = TinyGPS::course_to(lat,lon,Tlat,Tlon);
  /*Calculating the drive variable in two steps: 
  Step one is to calculate the change in angle needed to make heading equal to the course.
  Step two is to calculate the (propotional) drive by dividing the step one number with the change of angle in degrees that we want the drive to be maximum and clamp the result between the maximal and the minimal drive value
  0-90 is left 90-180 is right
  */
  drive = (course - heading) / 0.25;
  //drive = constrain(drive, -180 ,180);
  //-180 - Fully Winched In, turning ; 0 - Fully Winched Out, no turning.  
  SL.write( -(constrain(drive, -180 , 0)));
  //Rotates when SL at 0 an doesnt turn when it is on 0 itself; full whinch at +180  
  SR.write(constrain(drive, 0, 180));
 ////////////////////////////////Recieving commands from the ground\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
  
  if(Serial.available() > 1){
    String sIN = Serial.readString();
switch(sIN.charAt(0)){ //Commands: C ARG1 ARG2 ARG3 Example: s 1 
      case char('v'): //Setting Victim Coordinates 
          Tlat = sIN.substring(2, sIN.indexOf(' ')).toFloat();
          Tlon = sIN.substring(sIN.indexOf(' ')).toFloat();
          break;
      case char('i'): //Setting initial pressure and declination
          //initp = sIN.subString(2, sIN.indexOf(' ')).toFloat();
          declination = sIN.substring(sIN.indexOf(' ')).toFloat() * degtorad;
          break;
      case char('s'): //Enabling / Disabling servos (switch if not argumented , on if arg is 'ON' , off if anything else, Argument of max 2 chars) 
          if (sIN.length() > 1)
          {
            servoEN = sIN.substring(2,4) == "ON";
          } 
          else 
          {
            servoEN = !servoEN;
          }
          break;
        
          
          
    }
  }
}
