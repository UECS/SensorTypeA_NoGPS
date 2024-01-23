#include "MySHT3x.h"

SHT3x::SHT3x() {
}
//---------------------------------------------------------------毎秒１回呼ぶ関数
bool SHT3x::autoRead1Sec()
{
static bool SensorSts=false;
static bool measureFlag=false;
//------------------------------------------------------温湿度計測(2秒に一度)  
//SHT3xは計測コマンドの後、データが準備されるのに時間がかかる
//1秒間隔で計測コマンドと読み出しコマンドを交互に送信している

    if(!measureFlag)
      {
     //センサ計測開始
        startMeasure();
        measureFlag=true;
       }
    else if(getTempHumid())
      {
     //センサ読み出し成功
         double t=temp;
         double rh=humidity;
          //飽差計算
          double humidpress=6.1078*pow(10,(7.5*t/(t+237.3)));
          double humidvol=217*humidpress/(t+273.15);
          humiddiff=(100-rh)*humidvol/100;
          measureFlag=false;
          SensorSts=true;
        retryCount=0;
        }
     else
     {
     //センサ読み出し失敗
     measureFlag=false;
     SensorSts=false;
     humidity-999;
     temp=-999;
     humiddiff-999;
      if(!begin(SHT3x_ADDR))
          {begin(0x44);}
      if(retryCount<128){retryCount++;}
     }
  return SensorSts;
}

bool SHT3x::begin(unsigned char i2caddr) {
  Wire.begin();
  _i2caddr = i2caddr;
  return reset();
}

bool SHT3x::startMeasure(void)
{
int ret=writeCommand(0x2400);//MEAS_HIGHREP
if(ret==0){return true;}
  return false;
}


bool SHT3x::getTempHumid(void) {

  unsigned char i2cbuffer[6];
 
  Wire.requestFrom(_i2caddr, (unsigned char)6);
  if (Wire.available() != 6) 
    return false;
  for (unsigned char i=0; i<6; i++) {
    i2cbuffer[i] = Wire.read();
  }
  unsigned short SensorT, SensorRH;
  SensorT = i2cbuffer[0];
  SensorT <<= 8;
  SensorT |= i2cbuffer[1];

  if (i2cbuffer[2] != crc8Dallas(i2cbuffer, 2)) return false;

  SensorRH = i2cbuffer[3];
  SensorRH <<= 8;
  SensorRH |= i2cbuffer[4];

  if (i2cbuffer[5] != crc8Dallas(i2cbuffer+3, 2)) return false;

  temp = (double)SensorT*175.0/65535.0-45.0;
  humidity=(double)SensorRH*100.0/65535.0;

  return true;
}

bool SHT3x::reset(void) {
int ret=writeCommand(0x30A2);//reset command
  delay(10);
if(ret==0){return true;}
  return false;
}

unsigned char SHT3x::crc8Dallas(const unsigned char *data, int len) {
  unsigned char crcval(0xFF);
  
  for ( int j = len; j; --j ) {
      crcval ^= *data++;

      for ( int i = 8; i; --i ) {
  crcval = ( crcval & 0x80 )
    ? (crcval << 1) ^ 0x31//polynomial value
    : (crcval << 1);
      }
  }
  return crcval; 
}


int SHT3x::writeCommand(unsigned short cmd) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(cmd >> 8);
  Wire.write(cmd & 0xFF);
  return Wire.endTransmission();      
}
