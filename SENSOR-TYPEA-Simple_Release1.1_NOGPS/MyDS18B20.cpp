#include "MyDS18B20.h"


//温度測定コマンドを発行してから測定が完了するまで750msかかるので、偶数秒に測定コマンドを発行し、奇数秒に読み取る
//したがって、温度の更新は2秒間隔になる
double DS18B20::autoRead1Sec(char ch)
{
if(dssts[ch].measurecount>10){dssts[ch].measurecount=10;}
if(dssts[ch].errorcount>10){dssts[ch].errorcount=10;}
  
  switch (dssts[ch].count)
    {
    case 0:
       if(!dssts[ch].enabled)
        {
          OneWireSearch(ch);
        }
        else
       {
        OneWireSendStart(ch);
       }
      break;

    case 1:
        dssts[ch].TempTemp=OneWireGetTemp(ch); 
      break;
    }
  
  dssts[ch].count++;
  if(dssts[ch].count>1){dssts[ch].count=0;}
  
 if(!dssts[ch].enabled ||dssts[ch].measurecount<3)
    {return -999.9;}
    
  return dssts[ch].TempTemp;
  
}


//---------------------------------
double DS18B20::OneWireGetTemp(char ch)
{

  byte i;
  byte data[12];
  double celsius;

  if(ds[ch].reset()==0){
                        dssts[ch].enabled=false;
                        dssts[ch].errorcount++;
                        return -999.9;
                        }
      ds[ch].select(dssts[ch].addr);    
      ds[ch].write(0xBE);         // Read Scratchpad
    
      for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds[ch].read();
      }

  //CRCエラー

if(OneWire::crc8(data, 8) != data[8])
  {
    dssts[ch].measurecount=0;
    dssts[ch].errorcount++;
    return  -999.9;
    }


  int16_t raw = (data[1] << 8) | data[0];
  if (dssts[ch].type) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (double)raw / 16.0;
  //U_ccmList[dssts[ch].ccm].value=celsius;

 dssts[ch].measurecount++;
 dssts[ch].errorcount=0;

  return celsius;
  
  }
//---------------------------------
void DS18B20::OneWireSendStart(char ch)
{
    if(ds[ch].reset()==0)
        {
        dssts[ch].enabled=false;
        dssts[ch].errorcount++;
        dssts[ch].measurecount=0;
        return;
        }
  ds[ch].select(dssts[ch].addr);
  ds[ch].write(0x44,1);        // start conversion, with parasite power on at the end

  return;
 
  }

//---------------------------------
bool DS18B20::OneWireSearch(char ch)
{
    dssts[ch].measurecount=0;
  
//センサ探索
  if ( !ds[ch].search(dssts[ch].addr)) {ds[ch].reset_search();return false;}

//CRCエラー
if(OneWire::crc8(dssts[ch].addr, 7) != dssts[ch].addr[7])
  {
    dssts[ch].errorcount++;
    return false;
    }

//検出したチップによって分解能が違う
  switch (dssts[ch].addr[0]) {
    case 0x10:
    //Chip = DS18S20 or old DS1820
      dssts[ch].type = 1;
      break;
    case 0x28:
    //Chip = DS18B20
      dssts[ch].type = 0;
      break;
    case 0x22:
    //Chip = DS1822;
      dssts[ch].type = 0;
      break;
    default:
    //未知のデバイス
    return false;
  } 
  dssts[ch].enabled=true;
  dssts[ch].TempTemp=-999.9;
  dssts[ch].errorcount=0;
  return true;
  
}
