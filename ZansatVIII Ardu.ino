/*
//////////////////////////////////////////////////////////Info\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
                                       Zansat VIII cansat microcontroller code

                              Github Source: https://github.com/ZansatVIII/ZansatVIII-Arduino/
                          Author: Jakub Stachurski, Lead programmer @ the Zansat VIII Cansat Team 
                      
                        !! Disclaimer: This code is designed for a specific board and components, 
                        different hardware configurations may not result in optimal performance. !!
                       
              This version is not fully tested nor optimized, take this into account when using the code in any way: 
                                        The only test made is compiling this code 
                                           
                                          The code is licensed under GGPL v3.0:
                               (TL;DR : Plagiarizing this is fine as long as you admit to it)
                                    
                                        ... An output parsing tool coming soon ...

///////////////////////////////////////////////////////////Libs\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS.h>  
#include <QMC5883LCompass.h>
#include <SD.h>
#include <Servo.h>
/*///////////////////////////////////////////////////////Defines and vars\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

String out; 
static float pack [10] = {0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1000.0 , 0.0, 0.0 , 2.0};
static long lon , lat , Tlon, Tlat; //longtitude , lattitude , Target longtitude , Target lattitude
static unsigned long fix_age;
static bool servoEN = false; //Servo motors responsible for guidance are disabled at the start and can be enabled by command
static File Log ;
static Servo SL , SR;
/*/////////////PIN MAPPING\\\\\\\\\\\\\\\\\*/

#define GPSIN  8
#define GPSOUT 7
#define GPSPPS 3
#define SDSS 13
#define SRVL 5
#define SRVR 6

/*///////////PACK ARRAY MAPPING\\\\\\\\\\\\\
//First 5 values are sent, the first 4 are the bme readings */

#define TEMPS 0
#define PRESS 1
#define HEIGHT 2
#define HUMIDITY 3
#define HEADING 4
#define DRIVE 5
#define COURSE 6
#define INITP 7   
#define DECL 8
#define HEIGHTCHECK 9 

/*////////////EEPROM MAPPING\\\\\\\\\\\\\\\\
//(EEPROM usage is considered and not implemented yet) */

SoftwareSerial GPS(GPSIN,GPSOUT); //GPS 
TinyGPS gps;
Adafruit_BME280 bme; // I2C
QMC5883LCompass mag;

/*//////////////////////////////////////////////////Sends all data every interrupt/////////////////////////////
//Adds the float as string onto the out string */

void INIT(){ 
  if(SD.begin(SDSS)){
    File Log = SD.open("log.txt", FILE_WRITE);
    if(Log){
      out = 'S';
      }
    else {
      out = 'O';
    }
    SL.attach(SRVL);
    SR.attach(SRVR);
  }
}

void Tick(){
   
   pack [TEMPS] = bme.readTemperature(); 
   pack [PRESS] = bme.readPressure(); 
   pack [HEIGHT] = bme.readAltitude(pack[INITP]); 
   pack [HUMIDITY] = bme.readHumidity();
   out = out + String(lon, HEX);
   out = out + String(lat, HEX);
   if (servoEN) tone(2,1500,500);
   for(int i = 0; i <= 4; i ++){
     Serial.print(String(pack[i], 4));
     if(Log) {
       Log.print(String(pack[i], 4));
     }
   }
   
  Serial.print(out);
   if(Log) {
    Log.print(out);
  }
   out = "\n";
}
/*///////////////////////////////////////////Hardware and Library initialization\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
  pinMode(3,INPUT);
  pinMode(2,OUTPUT);
  //Initializes all logging and measurement capabilites
  INIT();
  if (!bme.begin()) {  
    while(1);
  }
  else{
    pack[INITP] = bme.readPressure();
  }
  mag.init();
  
  //Attaches interrupt to the pin recieving the pps signal
  attachInterrupt(digitalPinToInterrupt(GPSPPS), Tick , RISING);
}

void loop() {
  ///////////////////////////BEGIN OF LOOP | Orientation data gathering \\\\\\\\\\\\\\\\\\\\\\\
  /* FALLING CHECK // if heightcheck > 1  Or if the change in height is negative*/
  if (!pack[HEIGHTCHECK] || -1*(pack[HEIGHTCHECK] - pack[HEIGHT]))
  {
   if (pack[HEIGHTCHECK]) pack[HEIGHTCHECK] = -1.0;
   servoEN = true;
  }
  else pack[HEIGHTCHECK] = pack[HEIGHT]; servoEN = false;
  
  
  
  //Gets Longtitude And Lattitude from the gps
  while (GPS.available())
  {
    if (gps.encode(GPS.read()))
    {
    gps.get_position(&lat, &lon, &fix_age);
      
    }
  }
  //Reads calculates the direction we are heading (in degrees). TODO: Check Sensor orientation and change formula accordingly 
  pack[HEADING] = (atan2(mag.getY(), mag.getX())/ 57,29577) + pack[DECL]; 
  if(pack[HEADING] < 0){ //Check if the value is negative
    pack[HEADING] += 360;
  } 
  else if(pack[HEADING] > 360){ // Check for wrap due to addition of declination.
    pack[HEADING] -= 360;
  }
//////////////////////////////////////Drive Calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\  
  //Calculate Course (pack[5]) to target  N = 0 E = 90 S = 180 W = 270 , Own heading must obey the same rules
  pack[COURSE] = TinyGPS::course_to(lat,lon,Tlat,Tlon);
  /*Calculating the drive variable in two steps: 
  Step one is to calculate the change in angle needed to make heading equal to the course.
  Step two is to calculate the (propotional) drive by dividing the step one number with the change of angle in degrees that we want the drive to be maximum and clamp the result between the maximal and the minimal drive value
  0-90 is left 90-180 is right
  */
  pack[DRIVE] = (pack[COURSE] - pack[HEADING]) / 0.25;
  //drive = constrain(drive, -180 ,180);
  //-180 - Fully Winched In, turning ; 0 - Fully Winched Out, no turning.  
  SL.write( -(constrain(pack[DRIVE], -180 , 0)));
  //Rotates when SL at 0 an doesnt turn when it is on 0 itself; full whinch at +180  
  SR.write(constrain(pack[DRIVE], 0, 180));
 
  /*///////////////////////////////Recieving commands from the ground\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
  
  if(Serial.available() > 1){
    String sIN = Serial.readString();
switch(sIN.charAt(0)){ //Commands: C ARG1 ARG2 ARG3 Example: s 1 
      case char('v'): //Setting Victim Coordinates 
          Tlat = sIN.substring(2, sIN.indexOf(' ')).toInt();
          Tlon = sIN.substring(sIN.indexOf(' ')).toInt();
          break;
      case char('i'): //Setting initial pressure and declination
          pack[DECL] = sIN.substring(sIN.indexOf(' ')).toFloat();
          break;
      case char('s'): //Enabling / Disabling servos (switch if not argumented , on if arg is 'E' , off if anything else, Argument of max 2 chars) 
          if (sIN.length() > 2)
          {
            servoEN = sIN.substring(2,4) == 'E';
          } 
          else 
          {
            servoEN = !servoEN;
          }
          break;
        
          
          
    }
  }
}
