/*
  Chiptemp Library
  Created for the ZansatVIII project 
*/

// ensure this library description is only included once
#ifndef Chiptemp_h
#define Chiptemp_h

// library interface description
class Chiptemp
{
  // user-accessible "public" interface
  public:
    Chiptemp(int);
    void GetRead(void);

  // library-accessible "private" interface
  private:
    void Sample(void);
};

#endif
