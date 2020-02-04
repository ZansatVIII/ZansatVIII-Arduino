/*
Wirering:
APC220 > Arduino
gnd > gnd
vcc > D13
en > D12 Always high
RXD > D11
TXD > D10 
AUX > D9 Not in use 
SET > D8 Always high 
*/

#include <SoftwareSerial.h>

#define Rx  11
#define Tx  10
#define Vcc 13
#define En  12
#define Set 8
#define Aux 9

//Object creation
SoftwareSerial mySerial(Tx , Rx); 

void setup() {
  Serial.begin(9600);
  //Pin Init
  Serial.println("Pininit");
  pinMode(Set,OUTPUT);
  digitalWrite(Set,HIGH);
  pinMode(Vcc,OUTPUT);  // 5V
  digitalWrite(Vcc,HIGH); // turn on 5V
  delay(50);
  pinMode(En,OUTPUT); // ENABLE
  digitalWrite(En,HIGH); //
  delay(100);
  Serial.println("Businit");
  //Busses init 
  mySerial.begin(9600);
}

void loop() {
  //Bounces signals from both busses onto the other bus
  if(mySerial.available() > 1) Serial.println(mySerial.readString());
  else if(Serial.available() > 1) mySerial.println(Serial.readString()):

  delay(200);
}
