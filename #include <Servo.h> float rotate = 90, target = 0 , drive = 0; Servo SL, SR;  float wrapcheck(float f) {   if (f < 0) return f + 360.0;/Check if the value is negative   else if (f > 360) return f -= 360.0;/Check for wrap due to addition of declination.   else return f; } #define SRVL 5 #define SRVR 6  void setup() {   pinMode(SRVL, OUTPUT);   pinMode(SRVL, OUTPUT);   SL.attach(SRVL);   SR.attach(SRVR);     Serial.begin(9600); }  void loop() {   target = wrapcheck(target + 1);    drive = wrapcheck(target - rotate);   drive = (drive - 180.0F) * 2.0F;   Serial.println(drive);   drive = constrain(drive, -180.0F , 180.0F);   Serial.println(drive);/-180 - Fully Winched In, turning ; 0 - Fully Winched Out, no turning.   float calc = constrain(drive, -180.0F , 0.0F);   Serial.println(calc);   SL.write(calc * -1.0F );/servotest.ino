#include <Servo.h>
float rotate = 90, target = 0 , drive = 0;
Servo SL, SR;

float wrapcheck(float f) {
  if (f < 0) return f + 360.0; //Check if the value is negative
  else if (f > 360) return f -= 360.0; // Check for wrap due to addition of declination.
  else return f;
}
#define SRVL 5
#define SRVR 6

void setup() {
  pinMode(SRVL, OUTPUT);
  pinMode(SRVL, OUTPUT);
  SL.attach(SRVL);
  SR.attach(SRVR);  
  Serial.begin(9600);
}

void loop() {
  target = wrapcheck(target + 1); 
  drive = wrapcheck(target - rotate);
  drive = (drive - 180.0F) * 2.0F;
  Serial.println(drive);
  drive = constrain(drive, -180.0F , 180.0F);
  Serial.println(drive);
  //-180 - Fully Winched In, turning ; 0 - Fully Winched Out, no turning.
  float calc = constrain(drive, -180.0F , 0.0F);
  Serial.println(calc);
  SL.write(calc * -1.0F );
  //Rotates when SL at 0 an doesnt turn when it is on 0 itself; full whinch at +180
  calc =  constrain(drive, 0.0F, 180.0F);
  Serial.println(calc);
  SR.write(calc);

}
