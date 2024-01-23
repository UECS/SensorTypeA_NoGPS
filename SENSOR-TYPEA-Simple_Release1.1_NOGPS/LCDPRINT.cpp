#include "LCDPRINT.h"
#include "MyUardecs_mega.h"

//--------------------------------------------------------------
void LCDPRINT::begin()
{
lcd.begin();
lcd.init();
}
//---------------------------------------------------------------
void LCDPRINT::newline()
{
   lcd.command(0x40+0x80);//改行
}
//--------------------------------------------------------------
void LCDPRINT::clear()
{
   lcd.clear();//改行
}
//-------------------------------------------------------------
void LCDPRINT::printDateTime(int year,int month,int day,int hour,int min,int sec)
{      
  if(millis()/5000%2==0)
      {
        lcd.print(year);
        lcd.print("/");
        lcd.print(month);
        lcd.print("/");
        lcd.print(day);
      }
else
   {       
        lcd.print(hour);
        lcd.print(":");
        lcd.print(min);
        lcd.print(":");
        lcd.print(sec);
   }
}
//--------------------------------------
void LCDPRINT::printCO2Calibration(int co2CBCount)
{
  lcd.print("CO2 Calib:");
  lcd.print(co2CBCount);
  }
//--------------------------------------
void LCDPRINT::printFanStopError()
 {
  lcd.print("FAN STOP");
  }
//--------------------------------------
/*
 void LCDPRINT::printGPSCalib()
 {
  lcd.print("GPS calibration");
  //lcd.print("GPS \XBC\XDE\XAD\XBC\XDD\XCF\XC1");
  
  }
//--------------------------------------
 void LCDPRINT::printGPSError()
 {
  lcd.print("GPS Error");
  }
  */
//--------------------------------------
void LCDPRINT::printSHT31Error()
{
  lcd.print("SHT31 Error");
}
//--------------------------------------
 void LCDPRINT::printCO2Error()
 {
  lcd.print("S300 CO2 Error");
  }

//--------------------------------------
void LCDPRINT::printDS18B20Error(int ch)
{
  if(ch==0)
    {
    lcd.print("A3 TempSens Err");
    }
  
  if(ch==1)
    {
    lcd.print("A4 TempSens Err");
    }  
  
  }
//--------------------------------------
/*
void LCDPRINT::printRecvCount(int recvCCM,int recvCount)
{
        lcd.print("RecvCCM:");
        lcd.print(recvCount);
        lcd.print("/");
        lcd.print(recvCCM); 
}

void LCDPRINT::printCCM(int ccmid)
{
char ccmtype[MAX_CCMTYPESIZE];
strcpy(ccmtype,U_ccmList[ccmid].typeStr);
ccmtype[8]='\0';
lcd.print(U_ccmList[ccmid].baseAttribute[AT_ORDE]);
lcd.print(ccmtype);
lcd.print(":");

//値
if(!U_ccmList[ccmid].validity)
  {lcd.print("---");return;}

    if(U_ccmList[ccmid].decimal==0)//整数値の場合
        {lcd.print(U_ccmList[ccmid].value);}
    else
        {
        double realval=(double)(U_ccmList[ccmid].value)/pow(10,U_ccmList[ccmid].decimal);
        lcd.print(realval,U_ccmList[ccmid].decimal);
        }

 
}
*/
//--------------------------------------
void LCDPRINT::printSensors(double temp,double humid,double hd,double rpm,long co2,double radiation,double analog1,double analog2,double extemp1,double extemp2,int recvCCM,int recvCount)
{
static int count=0;
static int count_l=0;
static unsigned long lastsec=millis()/1000;
if(millis()/1000!=lastsec)
  {lastsec=millis()/1000;}
else{return;}
  
count++;
if(count>6)//1項目あたりの表示秒数
  {count=0;count_l++;}
count_l%=6;//表示項目数

//欠測した項目を飛ばす
for(int i=0;i<5;i++)
{
  switch(count_l)
  {
  case 0: 
          if(temp<-999&&humid<0){count_l++;break;}       
          break;
  case 1:
          if(hd<0 && rpm<0){count_l++;break;}       
          break;
  case 2:        
         if(co2<0 && radiation<0){count_l++;break;}       
         break;
  case 3:        
         if(analog1<0 && analog2<0){count_l++;break;}       
          break;
  case 4:        
          if(extemp1<-999 && extemp2<-999){count_l++;break;}       
          break;
  case 5:        
          break;
  }
}

//計測中の項目だけ表示する
switch(count_l)
{
case 0: 
        lcd.print("AirTemp Humidity");
        lcd.newLine();
        lcd.print(temp,1);
        lcd.print("C ");
        lcd.print(humid,1);
        lcd.print("%");
        break;
case 1:
        if(hd>=0)
          {
          lcd.print("HumidDiff ");
          }
        if(rpm>=0)
          {
          lcd.print("FANRPM");
          }
        lcd.newLine();
        if(hd>=0)
          {
          lcd.print(hd,1);
          lcd.print("g/m3 ");
          }
        if(rpm>=0)
          {
          lcd.print((int)rpm);
          lcd.print("FANRPM");
          }
        break;
case 2:
        if(co2>=0)
          {
           lcd.print("CO2 ");
          }
        if(radiation>=0)
          {
           lcd.print("SunRadiation");
          }
        lcd.newLine();
        if(co2>=0)
          {
           lcd.print(co2);
           lcd.print("ppm ");
          }
        if(radiation>=0)
          {
           lcd.print(radiation,2);
           lcd.print("kW/m2");
          }
       break;
case 3:        
        lcd.print("AnalogVoltage");
        lcd.newLine();
        if(analog1>=0)
        {
        lcd.print("1:");
        lcd.print(analog1,2);
        lcd.print("V 2:");
        }
        else
        {lcd.print("1:---V 2:");}

        if(analog2>=0)
        {
        lcd.print(analog2,2);
        lcd.print("V");
        }
        else
        {lcd.print("---V");}
        
        break;
case 4: 
        lcd.print("ExtrenalTempSens");
        lcd.newLine();
        if(extemp1>-999)
        {               
        lcd.print("3:");
        lcd.print(extemp1,1);
        lcd.print("C 4:");
        }
        else
        {
        lcd.print("3:---C 4:");
        }

        if(extemp2>-999)
        {
        lcd.print(extemp2,1);
        lcd.print("C");
        }
        else
        {
          lcd.print("---C");
        }
        
        break;

case 5:        
        /*
        lcd.print("RecvCCM:");
        lcd.print(recvCount);
        lcd.print("/");
        lcd.print(recvCCM);
        break;*/

        lcd.print("Sensor Type-AS");
        lcd.newLine();
        if(millis()/1000 %4==0)
          {lcd.print(".   .   .   .   ");}
        else if(millis()/1000 %4==1)
          {lcd.print(" o   o   o   o  ");}
        else if(millis()/1000 %4==2)
          {lcd.print("  O   O   O   O ");}
        else if(millis()/1000 %4==3)
          {lcd.print("   *   *   *   *");}
        break;
   }


   
}
//------------------------------------------
void LCDPRINT::printMyIP()
{
  clear();

  if(U_orgAttribute.status & STATUS_SAFEMODE)
  {
    lcd.print("192.168.1.7");
    newline();
    lcd.print("Safe Mode");
  }
else
  {
  lcd.print(U_orgAttribute.ip[0]);
  lcd.print(".");
  lcd.print(U_orgAttribute.ip[1]);
  lcd.print(".");
  lcd.print(U_orgAttribute.ip[2]);
  lcd.print(".");
  lcd.print(U_orgAttribute.ip[3]);
  newline();
  lcd.print(U_orgAttribute.subnet[0]);
  lcd.print(".");
  lcd.print(U_orgAttribute.subnet[1]);
  lcd.print(".");
  lcd.print(U_orgAttribute.subnet[2]);
  lcd.print(".");
  lcd.print(U_orgAttribute.subnet[3]);
  
  }
  
}
//------------------------------------------
/*
void LCDPRINT::printSdStatus(int writeFreq,bool SDEnable,int SDSts)
{
if(digitalRead(SDCARD_DITECT))
    {lcd.print("NOSD  ");return;}//SDカード入っていない
if(!digitalRead(SDCARD_RECSW))
    {lcd.print("STOP ");return;}//SDカードSW停止中
if(SDSts==SDCARD_ERROR_DENIED)
    {lcd.print("SDERR ");return;}//SDカードSW停止中
if(SDEnable)
    {
      lcd.print("REC");
      lcd.print(writeFreq);
      lcd.print("m ");
    }
  
}
*/
