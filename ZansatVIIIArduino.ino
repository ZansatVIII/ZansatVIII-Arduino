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
//#include <SPI.h> NOSD
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
//#include <TinyGPS.h> NOGPS 
#include <QMC5883LCompass.h>
//#include <SD.h> NOSD 
#include <Servo.h>
/*///////////////////////////////////////////////////////Defines and vars\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

String out;
float pack [10] = {0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1000.0 , 1013.25, 2.0 , 0.0};
long prevtick, detachtimer;
//static long lon , lat , Tlon, Tlat; //longtitude , lattitude , Target longtitude , Target lattitude
//static unsigned long fix_age;   GPS
bool servoEN = true, Auth = false, Bell = false; //Servo motors responsible for guidance are disabled at the start and can be enabled by command
// File Log ;
Servo SL , SR;
//Macro's 

#define TOG(x,y) (x^=(1<<y)) 

/*/////////////PIN MAPPING\\\\\\\\\\\\\\\\\*/

//#define GPSIN  8  //NOGPS
//#define GPSOUT 7
//#define GPSPPS 3
#define SDSS 13
#define BUZZ 2
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
#define INITP 7
#define DECL 8
#define HEIGHTCHECK 6
#define COURSE 9

/*////////////EEPROM MAPPING\\\\\\\\\\\\\\\\
  //(EEPROM usage is considered and not implemented yet) */

//SoftwareSerial GPS(GPSIN,GPSOUT); //NOGPS
//TinyGPS gps;
Adafruit_BME280 bme; // I2C
QMC5883LCompass mag;

/*//////////////////////////////////////////////////Sends all data every interrupt/////////////////////////////
  //Adds the float as string onto the out string */
/* NOSD 
void INIT() {
  if (SD.begin(SDSS)) {
    File Log = SD.open("log.txt", FILE_WRITE);
    if (Log) {
      out = 'S';
    }
    else {
      out = 'O';
    }
  }
}
*/
void Noise(){
  long i = millis()+300;
  while(i > millis()){
    TOG(PORTB,BUZZ);
    delayMicroseconds(200);
  }
}
void Tick() {
  /* FALLING CHECK // if heightcheck > 1  Or if the change in height is  more negative than -9m/s */
  if (((pack[HEIGHTCHECK] + 9.0) - pack[HEIGHT] ) < 0)
  {
    servoEN = true;
    pack[HEIGHTCHECK] = pack[HEIGHT];
    
  }
  else{
    pack[HEIGHTCHECK] = pack[HEIGHT];
    servoEN = false;
  }
  
  pack [TEMPS] = bme.readTemperature();
  pack [PRESS] = bme.readPressure() / 100.0F;
  pack [HEIGHT] = bme.readAltitude(pack[INITP]);
  pack [HUMIDITY] = bme.readHumidity();
  //out = out + String(lon, HEX);  //NOGPS
  //out = out + String(lat, HEX);
  Bell = Auth && servoEN;
  if (Bell || servoEN) {
    Noise();
  }
  for (int i = 0; i <= 6; i ++) {
    Serial.print(pack[i], 4);
    Serial.print(";");

   /* if (Log) { NOSD 
      Log.print(pack[i], 4);
      Log.print(";");
    }*/
  }
  Serial.print(servoEN);
  
  Serial.print(out);
  /*if (Log) { NOSD 
    Log.print(out);
  }*/
  out = "\n";
}
/*///////////////////////////////////////////Hardware and Library initialization\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void setup() {
  //GPS.begin(9600); NOGPS
  Serial.begin(9600);
  Serial.println(F("I"));
  pinMode(3, INPUT);
  pinMode(2, OUTPUT);
  Noise();
  //Initializes all logging and measurement capabilites
  //INIT(); NOSD 
  if (!bme.begin(0x76)) {
    Serial.println(F("NOBMP"));
  }
  else {
    pack[INITP] = bme.readPressure() / 100.0F + 1.0F;
  }
  pinMode(SRVL, OUTPUT);
  pinMode(SRVL, OUTPUT);
  SL.attach(SRVL);
  SR.attach(SRVR);
  mag.init();

  //Attaches interrupt to the pin recieving the pps signal
  //attachInterrupt(digitalPinToInterrupt(GPSPPS), Tick , RISING); //NOGPS
}

float wrapcheck(float f) {
  if (f < 0) return f + 360.0; //Check if the value is negative
  else if (f > 360) return f -= 360.0; // Check for wrap due to addition of declination.
  else return f;
}

void loop() {
  //Serial.println(F("LOOP"));
  ///////////////////////////BEGIN OF LOOP | Orientation data gathering \\\\\\\\\\\\\\\\\\\\\\\


  //Gets Longtitude And Lattitude from the gps
  /*while (GPS.available())   //NOGPS
    {
    if (gps.encode(GPS.read()))
    {
    gps.get_position(&lat, &lon, &fix_age);
    }
    }
  */
  //Reads calculates the direction we are heading (in degrees). TODO: Check Sensor orientation and change formula accordingly
  mag.read();
  pack[HEADING] = mag.getAzimuth() + pack[DECL];
  //pack[HEADING] = (atan2(mag.getY(), mag.getX())/ 57,29577) + pack[DECL]; //The old way using the raw data, may need to be recosidered depending on sensor orientation
  pack[HEADING] = wrapcheck(pack[HEADING]);

  //////////////////////////////////////Drive Calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\  
  //Calculate Course (pack[5]) to target  N = 0 E = 90 S = 180 W = 270 , Own heading must obey the same rules
  //pack[COURSE] = TinyGPS::course_to(lat,lon,Tlat,Tlon); //NOGPS
  /*Calculating the drive variable in two steps:
    Step one is to calculate the change in angle needed to make heading equal to the course.
    Step two is to calculate the (propotional) drive by dividing the step one number with the change of angle in degrees that we want the drive to be maximum and clamp the result between the maximal and the minimal drive value
    -180-0  is left 0 - 180 is right
  */
  pack[DRIVE] = wrapcheck(pack[COURSE] - pack[HEADING]); 
  pack[DRIVE] = (pack[DRIVE] - 180.0F) * 2.0F;
  pack[DRIVE] = constrain(pack[DRIVE], -180.0F , 180.0F);
  //-180 - Fully Winched In, turning ; 0 - Fully Winched Out, no turning.
  float calc = constrain(pack[DRIVE], -180.0F , 0.0F);
  SL.write(calc * -1.0F );
  //Rotates when SL at 0 an doesnt turn when it is on 0 itself; full whinch at +180
  calc =  constrain(pack[DRIVE], 0.0F, 180.0F);
  SR.write(calc);
  if (millis() - prevtick >= 1000) {
    Tick();
    prevtick = millis();
  }
  /*///////////////////////////////Recieving commands from the ground\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

  if (Serial.available() > 2) {
    String sIN = Serial.readString();
    switch (sIN[0]) { //Commands: C ARG1,ARG2;ARG3 Example: s 1
      /*case char('v'): //Setting Victim Coordinates NOGPS
        int spac = sIN.indexOf(',');
        Tlat = sIN.substring(2, spac).toInt();
        Tlon = sIN.substring(spac).toInt();
        break;
      */
      case char('p'): //Setting initial pressure and declination
        int ind;
        float flo;
        ind = sIN[2] - '0';
        flo = sIN.substring(4, 13).toFloat();
        pack[ind] = flo;
        Serial.print("var");
        Serial.print(ind);
        Serial.print("Changed to");
        Serial.print(flo, 4);
        break;

      case char('s'): //Enabling / Disabling servos (switch if not argumented , on if arg is 'E' , off if anything else, Argument of max 2 chars)
        if (sIN.length() > 2)
        {
          servoEN = sIN[2] == 'E';
        }
        else
        {
          servoEN = !servoEN;
        }
        break;
      case char('a'):
        Auth = true;
        break; 
    }
    Serial.println("COMMAND");
  }
}
