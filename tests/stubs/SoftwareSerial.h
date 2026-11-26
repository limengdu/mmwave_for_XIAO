#ifndef SoftwareSerial_h
#define SoftwareSerial_h

#include "Arduino.h"

class SoftwareSerial : public Stream
{
public:
    SoftwareSerial(int, int) {}
    int available() override { return 0; }
    int read() override { return -1; }
};

#endif
