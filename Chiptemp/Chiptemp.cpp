
#include "Chiptemp.h"

//INIT 
void Chiptemp:Chiptemp(int s){
  samplesize = s
 
  //Copied Code
  ADCSRA |= _BV(ADSC); // start the conversion
  while (bit_is_set(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes
  return (ADCL | (ADCH << 8)) - 342; // combine bytes & correct for temp offset (approximate)}
  
}
//read temperature once 
int Chiptemp::Sample()
{
 //Copied code
 ADCSRA |= _BV(ADSC); // start the conversion
 while (bit_is_set(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes
 return (ADCL | (ADCH << 8)) - 342; // combine bytes & correct for temp offset (approximate)}
}

//Average the temperatures 
float Chiptemp::GetRead(){
  //one reading discarded, safety feature 
  Sample();
  float current ;
  //copied code
  for (int i = 1; i < samplesize; i++) // start at 1 so we dont divide by 0
    current += ((Sample() - current)/(float)i);
  
  return current;
}
