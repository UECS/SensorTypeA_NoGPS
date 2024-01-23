#include "MyAQM1602.h"

MyAQM1602::MyAQM1602(uint8_t addr)
{
  _i2caddress = addr;
  Wire.begin();
}

void MyAQM1602::command(uint8_t value)
{
  send(value, (uint8_t)0x00);
}

size_t MyAQM1602::write(uint8_t value)
{
  send(value, (uint8_t)0x40);
  return 1;
}


void MyAQM1602::send(uint8_t value, uint8_t mode)
{
  Wire.beginTransmission(_i2caddress);
  Wire.write(mode); 
  Wire.write(value);
  Wire.endTransmission();
  delayMicroseconds(27);
}


void MyAQM1602::init()
{
  begin();
}

void MyAQM1602::begin()
{

  delay(40);
  /*
  command(0x38);
  command(0x39);
  command(0x14);
  command(0x70);
  command(0x56);
  command(0x6C);
  command(0x38);*/
  command(0x38);delay(1);
  command(0x39);delay(1);
  command(0x14);delay(1);
  command(0x73);delay(1);
  command(0x51);delay(2);
  command(0x6c);delay(300);
  command(0x38);delay(1);
  command(0x01);delay(2);
  command(0x0c);
  
  _displayflags = AQM1602CMD_DISPLAYON | AQM1602CMD_CURSOROFF | AQM1602CMD_BLINKOFF;
  displayOn();
  clear();
  home();
}
void MyAQM1602::displayOn()
{
  _displayflags |= AQM1602CMD_DISPLAYON;
  command(AQM1602CMD_DISPLAYCONTROL | _displayflags);
}
void MyAQM1602::home()
{
  command(AQM1602CMD_RETURNHOME);
  delayMicroseconds(2000);
}
void MyAQM1602::clear()
{
  command(AQM1602CMD_CLEARDISPLAY);
  delayMicroseconds(2000);
}

void MyAQM1602::newLine()
{
command(0x40+0x80);
}