#ifndef _LCDPRINT_H_
#define _LCDPRINT_H_

#include "Arduino.h"
#include "MyAQM1602.h"
#include "string.h"


class LCDPRINT {
  public:
 void begin();
 void clear();
 void printDateTime(int year,int month,int day,int hour,int min,int sec);
// void printGPSError();
// void printGPSCalib();
// void printSdStatus(int writeFreq,bool SDEnable,int SDSts);
 void printSHT31Error();
 void printFanStopError();
 void printCO2Error();
 void printCO2Calibration(int co2CBCount);
 void printSensors(double,double,double,double,long,double,double,double,double,double,int,int);
 void printDS18B20Error(int ch);
// void printRecvCount(int,int);
// void printCCM(int ccmid);
 void printMyIP();
 void newline();
 
 private:
 MyAQM1602 lcd;
};


#endif
