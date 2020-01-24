/*
  Chiptemp Library
  Created for the ZansatVIII project 
*/

// ensure this library description is only included once
#ifndef Chiptemp_h
#define Chiptemp_h

#include <WProgram.h>

// library interface description
class Chiptemp
{
  // user-accessible "public" interface
  public:
    //Constructor , requires sample size
    Chiptemp(int s);
    //Temperature returning function
    float GetRead(void);

  // library-accessible "private" interface
  private:
    int samplesize;
    int Sample(void);
};

#endif
