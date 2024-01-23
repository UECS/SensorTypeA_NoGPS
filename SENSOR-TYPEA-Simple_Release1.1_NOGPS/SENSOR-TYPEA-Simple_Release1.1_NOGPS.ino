//2023年10月27日 Ver1.0 GPS・SDカード無し版
//[概要]UECS対応センサユニットAS型 ソースコード
//気温,湿度,飽差,CO2,日射,温度2系統,アナログ電圧2系統
//Mammillaria Shield Arduino MEGA専用

//使用条件
//書き換えが必要なハードコーディングされた行があります
//★マークを検索して下さい

/*
TYPE-AとTYPE-ASの違い
・TYPE-A
　フル機能バージョン、GPSによる時計機能とSDカードへのデータ記録が可能
・TYPE-AS
　ソフトウェア違いによる機能限定バージョン、GPSとSDカード記録機能を廃止(それらのモジュールも実装不要)
　外部CCM受信機能も廃止(SDカードに記録しないため)
　データの記録には外部に何らかの装置(PCやクラウドなど)が必要です。
*/

/*
ファームウェア書き込み時の注意:
最初の1回目は基板に装着する前のArduinoMEGAをPCにUSBケーブルで直結して書き込んで下さい。
(もしArduinoを他の用途に使っていた場合、誤作動して基板を破壊する危険性があります)
2回目以降は基板に装着したままファームウェア書き込むことが可能ですが、
基板装着状態のArduinoはUSBケーブルからの電源供給だけでは不足します(部品を損傷します)。
必ずACアダプタを基板上のDCジャックに接続して下さい。
Arduino側のDCジャックでは十分な電力供給ができません。必ず基板上のDCジャックを使って下さい。
工場出荷時のArduinoはEEPROMに無意味な値が書き込まれており、そのままではセンサが正常に動作しません。
LANケーブルでPCに接続しブラウザから初期設定を行い、値を初期化する必要があります。
*/


#include <SPI.h>
#include <MyEthernet2.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <string.h>
#include <math.h>
#include "MyUardecs_mega.h"
#include "s300i2c.h"
#include "Mysht3x.h"
#include <OneWire.h>
#include "MyDS18B20.h"
#include "LCDPRINT.h"
//#include "SDWRITE.h"

/*ライセンスの範囲：
このファイル
LCDPRINT.h
LCDPRINT.cpp
*/

/* 
使用しているオープンソースライブラリ一覧
Arduino.h       Arduino IDE付属
SPI.h		        Arduino IDE付属
avr/pgmspace.h	Arduino IDE付属
EEPROM.h	      Arduino IDE付属
Wire.h          Arduino IDE付属
Print.h         Arduino IDE付属
string.h        Arduino IDE付属
math.h          Arduino IDE付属
string.h        Arduino IDE付属
MyGPS.h		      https://github.com/UECS/MyGPS
Mysht3x.h	      https://github.com/UECS/Mysht3x
MyDS18B20.h     https://github.com/UECS/MyDS18B20
MyAQM1602.h	    https://github.com/UECS/MyAQM1602
OneWire.h        https://github.com/PaulStoffregen/OneWire      ※このライブラリは\Documents\Arduino\libraries下に置く必要がある
s300i2c.h        https://github.com/mhorimoto/ELT_S300_HOLLY    ※このライブラリは\Documents\Arduino\libraries下に置く必要がある
MyEthernet2.h     https://github.com/H-Kurosaki/MyEthernet2     ※このライブラリは\Documents\Arduino\libraries下に置く必要がある
MyUardecs_mega.h  https://github.com/H-Kurosaki/MyUardecs_mega  ※このライブラリは\Documents\Arduino\libraries下に置く必要がある
*/

//Arduinoピン番号設定
#define PIN_SPK        6
#define PIN_WDT        7
#define PIN_FANRPM    8
#define PIN_MCDL      9

//CNDステータスコード
#define GPS_POSNOTFOUND   0x80000     //GPSで位置情報の十分な精度が得られない
#define GPS_NOTCALIBRATED 0x100000    //GPSがキャリブレーション中
#define GPS_DISCONNTECT   0x200000    //GPSが見つからない
#define CO2_CALIBLATION   0x000001    //CO2がキャリブレーション中

#define OPRMODE_ERR_SHT3xSENSERR           0x4000000  //0 00001 00000 0 0000 0000000000000000 //温湿度センサエラー
#define OPRMODE_ERR_FANSTOP                0x8000000  //0 00010 00000 0 0000 0000000000000000 //ファンが止まった
#define OPRMODE_ERR_S300CO2                0x10000000 //0 00010 00000 0 0000 0000000000000000 //CO2センサが認識できない
#define OPRMODE_ERR_DS18B20                0x20000000 //0 01000 00000 0 0000 0000000000000000 //防水温度センサと通信できない
//#define OPRMODE_ERR_SDERROR                0x40000000 //0 10000 00000 0 0000 0000000000000000 //SDカードエラー
//#define OPRMODE_ERR            0x40000000 //0 10000 00000 0 0000 0000000000000000 

//#define SD_WRITE_FREQMIN  5//SDカード記録間隔(分)


//各種センサ初期化
//GPS AeGPS;//GPS
LCDPRINT LCD;//液晶
//SDWRITE SDCard=SDWRITE(4,10);//SDカードのCS：pin4,LANのCS:pin10
SHT3x sht3x = SHT3x();//温湿度センサ
S300I2C CO2S300(Wire);//CO2センサ
DS18B20 ExTempSens;//使用ピン設定はMyDS18B20.hの中

char bootTime=0;//起動直後(設定変更直後)に不安定なセンサをエラーと見なさないようにするカウンタ

/////////////////////////////////////
//IP reset jupmer pin setting
//IPアドレスリセット用ジャンパーピン設定
/////////////////////////////////////
const byte U_InitPin = 3;
const byte U_InitPin_Sense=LOW;  

////////////////////////////////////
//Node basic infomation
///////////////////////////////////
//★以下の５つの文字列の初期値はベンダーが書き換えて下さい
const char U_name[] PROGMEM= "TYPE-AS";//MAX 20 chars
const char U_vender[] PROGMEM= "WARC/NARO";//MAX 20 chars
const char U_uecsid[] PROGMEM= "000000000000";//12 chars fixed UECS研究会が割り当てたIDの記述が必要です
const char U_footnote[] PROGMEM= "UECS Sensor TYPE-A Simplified Ver1.0B";
char U_nodename[20] = "Sensor";//MAX 19chars
UECSOriginalAttribute U_orgAttribute;

//////////////////////////////////
// html page1 setting
//////////////////////////////////
const int U_HtmlLine = 9; //Total number of HTML table rows.


//●表示素材の定義 選択肢表示
const char NONES[] PROGMEM= "";
const char** DUMMY = NULL;

const char UECSOFF[] PROGMEM= "停止";
const char UECSON[] PROGMEM= "使う";

//UECSSELECTDATA
const char *stringSELECT[2]={
UECSOFF,
UECSON,
};



//const char NAME0[] PROGMEM= "日付時刻送信";
//const char NOTE0[] PROGMEM= "時計信号を送信します";
//signed long setDATETIMEONOFF;

//const char NAME1[] PROGMEM= "GPS情報送信";
//const char NOTE1[] PROGMEM= "衛星を補足しないと送信されません";
//signed long setGPSONOFF;

const char NAME2[] PROGMEM= "温湿度(飽差)センサ";
const char NOTE2[] PROGMEM= "応答に数秒かかります";
signed long setSHT31ONOFF;

const char NAME3[] PROGMEM= "FAN回転数";
const char NOTE3[] PROGMEM= "FAN停止時にエラーを出します";
signed long setFANONOFF;

const char NAME4[] PROGMEM= "CO2センサ";
const char NOTE4[] PROGMEM= "応答に数秒かかります";
signed long setS300ONOFF;

const char NAME5[] PROGMEM= "日射センサ";
const char NOTE5[] PROGMEM= "注:断線時に警報は出ません";
signed long setSUNONOFF;

const char NAME6[] PROGMEM= "アナログ入力A1";
const char NOTE6[] PROGMEM= "注:断線時に警報は出ません";
signed long setADC1ONOFF;

const char NAME7[] PROGMEM= "アナログ入力A2";
const char NOTE7[] PROGMEM= "注:断線時に警報は出ません";
signed long setADC2ONOFF;


const char NAME8[] PROGMEM= "防水温度計1(A3)";
const char NOTE8[] PROGMEM= "応答に数秒かかります";
signed long setA3ONOFF;

const char NAME9[] PROGMEM= "防水温度計2(A4)";
const char NOTE9[] PROGMEM= "応答に数秒かかります";
signed long setA4ONOFF;

/*
const char NAME_SD[] PROGMEM= "SDカード状態";
const char NOTE_SD[] PROGMEM= "FAT32フォーマットのみ書き込み可能";
const char SHOWSTRING_SDOK[] PROGMEM= "記録中";
const char SHOWSTRING_NOSD [] PROGMEM= "カード無し";
const char SHOWSTRING_SDERR [] PROGMEM= "エラー";
const char SHOWSTRING_SDSTOP [] PROGMEM= "記録停止中";
const char *stringSHOW[4]={
SHOWSTRING_SDOK,
SHOWSTRING_NOSD,
SHOWSTRING_SDERR,
SHOWSTRING_SDSTOP,
};
signed long statusSD;
*/

const char UECSNONE[] PROGMEM= "何もしない";
const char UECSSTART[] PROGMEM= "校正実行";
const char *stringEXESELECT[2]={
UECSNONE,
UECSSTART,
};

const char NAMECO2CB[] PROGMEM= "CO2校正";
const char NOTECO2CB[] PROGMEM= "10分かかります";
signed long setEXESELECT;
int co2CBCount=0;

struct UECSUserHtml U_html[U_HtmlLine]={
//  {NAME0,  UECSSELECTDATA  ,NONES  ,NOTE0, stringSELECT  , 2 , &(setDATETIMEONOFF) , 0, 0, 0},
//  {NAME1,  UECSSELECTDATA  ,NONES  ,NOTE1, stringSELECT  , 2 , &(setGPSONOFF) , 0, 0, 0},
  {NAME2,  UECSSELECTDATA  ,NONES  ,NOTE2, stringSELECT  , 2 , &(setSHT31ONOFF) , 0, 0, 0},
  {NAME3,  UECSSELECTDATA  ,NONES  ,NOTE3, stringSELECT  , 2 , &(setFANONOFF) , 0, 0, 0},
  {NAME4,  UECSSELECTDATA  ,NONES  ,NOTE4, stringSELECT  , 2 , &(setS300ONOFF) , 0, 0, 0},
  {NAME5,  UECSSELECTDATA  ,NONES  ,NOTE5, stringSELECT  , 2 , &(setSUNONOFF) , 0, 0, 0},
  {NAME6,  UECSSELECTDATA  ,NONES  ,NOTE6, stringSELECT  , 2 , &(setADC1ONOFF) , 0, 0, 0},
  {NAME7,  UECSSELECTDATA  ,NONES  ,NOTE7, stringSELECT  , 2 , &(setADC2ONOFF) , 0, 0, 0},
  {NAME8,  UECSSELECTDATA  ,NONES  ,NOTE8, stringSELECT  , 2 , &(setA3ONOFF) , 0, 0, 0},
  {NAME9,  UECSSELECTDATA  ,NONES  ,NOTE9, stringSELECT  , 2 , &(setA4ONOFF) , 0, 0, 0}, 
//  {NAME_SD,  UECSSHOWSTRING  ,NONES  ,NOTE_SD, stringSHOW  , 4 , &(statusSD) , 0, 0, 0},
  {NAMECO2CB,  UECSSELECTDATA  ,NONES  ,NOTECO2CB, stringEXESELECT  , 2 , &(setEXESELECT) , 0, 0, 0},
 };

//////////////////////////////////
// UserCCM setting
//////////////////////////////////

//define CCMID for identify
//CCMID_dummy must put on last
enum {
//CCMID_Date,
//CCMID_Time,
CCMID_cnd,
//CCMID_Lat,
//CCMID_Lon,
//CCMID_Sat,
CCMID_FanRPM,
CCMID_InAirTemp,
CCMID_InAirHumid,
CCMID_InAirHD,
CCMID_InAirCO2,
CCMID_InRadiation,
CCMID_Volt1,
CCMID_Volt2,
CCMID_ExTemp1,
CCMID_ExTemp2,
//CCMID_InAirTempEx,
//CCMID_InAirHumidEx,
//CCMID_InAirHDEx,
//CCMID_InAirCO2Ex,
//CCMID_InRadiationEx,
//CCMID_Volt1Ex,
//CCMID_Volt2Ex,
//CCMID_ExTemp1Ex,
//CCMID_ExTemp2Ex,
//CCMID_FlowEx,
CCMID_dummy,
};

const int U_MAX_CCM = CCMID_dummy;
UECSCCM U_ccmList[U_MAX_CCM];
//bool sensorUseFlag[U_MAX_CCM];
/*
const char ccmNameDate[] PROGMEM= "Date";
const char ccmTypeDate[] PROGMEM= "Date";
const char ccmUnitDate[] PROGMEM= "yymmdd";

const char ccmNameTime[] PROGMEM= "Time";
const char ccmTypeTime[] PROGMEM= "Time";
const char ccmUnitTime[] PROGMEM= "hhmmss";
*/
const char ccmNameCnd[] PROGMEM= "状態";
const char ccmTypeCnd[] PROGMEM= "cnd.mIC";
const char ccmUnitCnd[] PROGMEM= "";

const char ccmNameLat[] PROGMEM= "経度";
const char ccmTypeLat[] PROGMEM= "Latitude.mIC";
const char ccmUnitLat[] PROGMEM= "degree";

const char ccmNameLon[] PROGMEM= "緯度";
const char ccmTypeLon[] PROGMEM= "Longitude.mIC";
const char ccmUnitLon[] PROGMEM= "degree";

const char ccmNameSat[] PROGMEM= "捕捉衛星";
const char ccmTypeSat[] PROGMEM= "Satellites.mIC";
const char ccmUnitSat[] PROGMEM= "";

const char ccmNameRPM[] PROGMEM= "FAN回転数";
const char ccmTypeRPM[] PROGMEM= "FanRPM.mIC";
const char ccmUnitRPM[] PROGMEM= "RPM";

const char ccmNameTemp[] PROGMEM= "気温";
const char ccmTypeTemp[] PROGMEM= "InAirTemp.mIC";
const char ccmUnitTemp[] PROGMEM= "C";

const char ccmNameHumid[] PROGMEM= "湿度";
const char ccmTypeHumid[] PROGMEM= "InAirHumid.mIC";
const char ccmUnitHumid[] PROGMEM= "%";

const char ccmNameHD[] PROGMEM= "飽差";
const char ccmTypeHD[] PROGMEM= "InAirHD.mIC";
const char ccmUnitHD[] PROGMEM= "g m-3";

const char ccmNameCO2[] PROGMEM= "CO2";
const char ccmTypeCO2[] PROGMEM= "InAirCO2.mIC";
const char ccmUnitCO2[] PROGMEM= "ppm";

const char ccmNameSUN[] PROGMEM= "日射";
const char ccmTypeSUN[] PROGMEM= "InRadiation.mIC";
const char ccmUnitSUN[] PROGMEM= "kW m-2";

const char ccmNameADC1[] PROGMEM= "Analog1(A1)";
const char ccmTypeADC1[] PROGMEM= "Volt.1.mIC";
const char ccmUnitADC1[] PROGMEM= "V";

const char ccmNameADC2[] PROGMEM= "Analog2(A2)";
const char ccmTypeADC2[] PROGMEM= "Volt.2.mIC";
const char ccmUnitADC2[] PROGMEM= "V";

const char ccmNameExTemp1[] PROGMEM= "防水温度計1(A3)";
const char ccmTypeExTemp1[] PROGMEM= "SoilTemp.1.mIC";
const char ccmUnitExTemp1[] PROGMEM= "C";

const char ccmNameExTemp2[] PROGMEM= "防水温度計2(A4)";
const char ccmTypeExTemp2[] PROGMEM= "SoilTemp.2.mIC";
const char ccmUnitExTemp2[] PROGMEM= "C";
/*
const char ccmNameTempEx[] PROGMEM= "受信気温";
const char ccmTypeTempEx[] PROGMEM= "InAirTemp.mIC";
const char ccmUnitTempEx[] PROGMEM= "C";

const char ccmNameHumidEx[] PROGMEM= "受信湿度";
const char ccmTypeHumidEx[] PROGMEM= "InAirHumid.mIC";
const char ccmUnitHumidEx[] PROGMEM= "%";

const char ccmNameHDEx[] PROGMEM= "受信飽差";
const char ccmTypeHDEx[] PROGMEM= "InAirHD.mIC";
const char ccmUnitHDEx[] PROGMEM= "g m-3";

const char ccmNameCO2Ex[] PROGMEM= "受信CO2";
const char ccmTypeCO2Ex[] PROGMEM= "InAirCO2.mIC";
const char ccmUnitCO2Ex[] PROGMEM= "ppm";

const char ccmNameSUNEx[] PROGMEM= "受信日射";
const char ccmTypeSUNEx[] PROGMEM= "InRadiation.mIC";
const char ccmUnitSUNEx[] PROGMEM= "kW m-2";

const char ccmNameADC1Ex[] PROGMEM= "受信Analog1";
const char ccmTypeADC1Ex[] PROGMEM= "Volt.1.mIC";
const char ccmUnitADC1Ex[] PROGMEM= "V";

const char ccmNameADC2Ex[] PROGMEM= "受信Analog2";
const char ccmTypeADC2Ex[] PROGMEM= "Volt.2.mIC";
const char ccmUnitADC2Ex[] PROGMEM= "V";

const char ccmNameExTemp1Ex[] PROGMEM= "受信防水温度計1";
const char ccmTypeExTemp1Ex[] PROGMEM= "SoilTemp.1.mIC";
const char ccmUnitExTemp1Ex[] PROGMEM= "C";

const char ccmNameExTemp2Ex[] PROGMEM= "受信防水温度計2";
const char ccmTypeExTemp2Ex[] PROGMEM= "SoilTemp.2.mIC";
const char ccmUnitExTemp2Ex[] PROGMEM= "C";


const char ccmNameFlowEx[] PROGMEM= "受信流量計";
const char ccmTypeFlowEx[] PROGMEM= "Flow.mIC";
const char ccmUnitFlowEx[] PROGMEM= "L";
*/

void UserInit(){
//6つのMACアドレスはW550ioのシールに書かれたものに書き換えて下さい
//ただしMACアドレス自動設定ライブラリを使用している場合は設定値は無視されW5500内部に記録されたアドレスが反映されます
U_orgAttribute.mac[0] = 0x00;
U_orgAttribute.mac[1] = 0x00;
U_orgAttribute.mac[2] = 0x00;
U_orgAttribute.mac[3] = 0x00;
U_orgAttribute.mac[4] = 0x00;
U_orgAttribute.mac[5] = 0x01;

//Set ccm list
//UECSsetCCM(true, CCMID_Date, ccmNameDate, ccmTypeDate, ccmUnitDate, 29, 0, A_1M_0);
//UECSsetCCM(true, CCMID_Time, ccmNameTime, ccmTypeTime, ccmUnitTime, 29, 0, A_1S_0);
UECSsetCCM(true, CCMID_cnd, ccmNameCnd , ccmTypeCnd , ccmUnitCnd , 29, 0, A_10S_0);
//UECSsetCCM(true, CCMID_Lat, ccmNameLat, ccmTypeLat, ccmUnitLat, 29, 6, A_1M_0);
//UECSsetCCM(true, CCMID_Lon, ccmNameLon, ccmTypeLon, ccmUnitLon, 29, 6, A_1M_0);
//UECSsetCCM(true, CCMID_Sat, ccmNameSat, ccmTypeSat, ccmUnitSat, 29, 0, A_1M_0);
UECSsetCCM(true, CCMID_FanRPM, ccmNameRPM, ccmTypeRPM, ccmUnitRPM, 29, 0, A_1M_0);
UECSsetCCM(true, CCMID_InAirTemp, ccmNameTemp, ccmTypeTemp, ccmUnitTemp, 29, 1, A_10S_0);
UECSsetCCM(true, CCMID_InAirHumid, ccmNameHumid, ccmTypeHumid, ccmUnitHumid, 29, 1, A_10S_0);
UECSsetCCM(true, CCMID_InAirHD, ccmNameHD, ccmTypeHD, ccmUnitHD, 29, 2, A_10S_0);
UECSsetCCM(true, CCMID_InAirCO2, ccmNameCO2, ccmTypeCO2, ccmUnitCO2, 29, 0, A_10S_0);
UECSsetCCM(true, CCMID_InRadiation, ccmNameSUN, ccmTypeSUN, ccmUnitSUN, 29, 3, A_10S_0);
UECSsetCCM(true, CCMID_Volt1, ccmNameADC1, ccmTypeADC1, ccmUnitADC1, 29, 3, A_10S_0);
UECSsetCCM(true, CCMID_Volt2, ccmNameADC2, ccmTypeADC2, ccmUnitADC2, 29, 3, A_10S_0);
UECSsetCCM(true, CCMID_ExTemp1, ccmNameExTemp1, ccmTypeExTemp1, ccmUnitExTemp1, 29, 1, A_10S_0);
UECSsetCCM(true, CCMID_ExTemp2, ccmNameExTemp2, ccmTypeExTemp2, ccmUnitExTemp2, 29, 1, A_10S_0);
/*
UECSsetCCM(false, CCMID_InAirTempEx, ccmNameTempEx, ccmTypeTempEx, ccmUnitTempEx, 29, 1, A_1M_0);
UECSsetCCM(false, CCMID_InAirHumidEx, ccmNameHumidEx, ccmTypeHumidEx, ccmUnitHumidEx, 29, 1, A_1M_0);
UECSsetCCM(false, CCMID_InAirHDEx, ccmNameHDEx, ccmTypeHDEx, ccmUnitHDEx, 29, 2, A_1M_0);
UECSsetCCM(false, CCMID_InAirCO2Ex, ccmNameCO2Ex, ccmTypeCO2Ex, ccmUnitCO2Ex, 29, 0, A_1M_0);
UECSsetCCM(false, CCMID_InRadiationEx, ccmNameSUNEx, ccmTypeSUNEx, ccmUnitSUNEx, 29, 3, A_1M_0);
UECSsetCCM(false, CCMID_Volt1Ex, ccmNameADC1Ex, ccmTypeADC1Ex, ccmUnitADC1, 29, 3, A_1M_0);
UECSsetCCM(false, CCMID_Volt2Ex, ccmNameADC2Ex, ccmTypeADC2Ex, ccmUnitADC2, 29, 3, A_1M_0);
UECSsetCCM(false, CCMID_ExTemp1Ex, ccmNameExTemp1Ex, ccmTypeExTemp1Ex, ccmUnitExTemp1, 29, 1, A_1M_0);
UECSsetCCM(false, CCMID_ExTemp2Ex, ccmNameExTemp2Ex, ccmTypeExTemp2Ex, ccmUnitExTemp2, 29, 1, A_1M_0);
UECSsetCCM(false, CCMID_FlowEx, ccmNameFlowEx, ccmTypeFlowEx, ccmUnitFlowEx, 29, 0, A_1M_0);
*/
}

//Webページからの入力を受け付ける
void OnWebFormRecieved(){
int i;
bootTime=0;//カウンタリセット


if(setEXESELECT)
    {
      setEXESELECT=0;
      if(setS300ONOFF)
        {
          co2CBCount=600;
          pinMode(PIN_MCDL,OUTPUT);
          digitalWrite(PIN_MCDL,LOW);
        }
    }

}

void UserEverySecond(){
if(bootTime<60){bootTime++;}


  
  U_ccmList[CCMID_cnd].value=0;


//////////////////////////////////////////////////SHT31
//////////////////////////////////////////////////

bool sht31ret=false;
if(setSHT31ONOFF)
    {
        sht31ret=sht3x.autoRead1Sec();
        if(sht31ret)
        {
          U_ccmList[CCMID_InAirTemp].value=(long)(sht3x.temp*10);
          U_ccmList[CCMID_InAirHumid].value=(long)(sht3x.humidity*10);
          U_ccmList[CCMID_InAirHD].value=(long)(sht3x.humiddiff*100);
          }
    }
if(!sht31ret || !setSHT31ONOFF || bootTime<5)
        {
        U_ccmList[CCMID_InAirTemp].value=-9999;
        U_ccmList[CCMID_InAirHumid].value=-9999;
        U_ccmList[CCMID_InAirHD].value=-99999;
        U_ccmList[CCMID_InAirTemp].flagStimeRfirst=false;
        U_ccmList[CCMID_InAirHumid].flagStimeRfirst=false;
        U_ccmList[CCMID_InAirHD].flagStimeRfirst=false;
        }

//////////////////////////////////////////////////FAN
//////////////////////////////////////////////////
double rpm=0;
static char fanErrorCount=0;
if(setFANONOFF)
{
rpm=CountFanRPM();
U_ccmList[CCMID_FanRPM].value=(long)rpm;
if(rpm==0&&fanErrorCount<10){fanErrorCount++;}
else{fanErrorCount=0;}
}
else
{
  fanErrorCount=0;
  rpm=-999;
  U_ccmList[CCMID_FanRPM].value=-999;
  U_ccmList[CCMID_FanRPM].flagStimeRfirst=false;
  }
//////////////////////////////////////////////////CO2
//////////////////////////////////////////////////
unsigned int co2=0;
    if(co2CBCount>0)//校正モード
    {
        co2CBCount--;
        if(co2CBCount==0)
            {
              pinMode(PIN_MCDL,INPUT);
             }
    }

    if(setS300ONOFF)
    {
    co2 = CO2S300.getCO2ppm();
    U_ccmList[CCMID_InAirCO2].value=co2;
    }
 
    //CO2センサがない場合、データ送信を停止する
    if(co2==0)
      {
        U_ccmList[CCMID_InAirCO2].value=-999;
        U_ccmList[CCMID_InAirCO2].flagStimeRfirst=false;
        if(co2CBCount>0)
            {
              pinMode(PIN_MCDL,INPUT);
              co2CBCount=0;
            }
       }
    
//////////////////////////////////////////////////SUN
//////////////////////////////////////////////////
double radiation=0;
if(setSUNONOFF&&bootTime>3)
{
radiation=GetAveragedAnalogInputA0();
//1.1Vモード換算式
//radiation= radiation*1.0/930.0;
//5Vモード換算式
radiation= radiation/204.6;
U_ccmList[CCMID_InRadiation].value=(long)(radiation*1000);
}
else
{
  radiation=-9.99;
  U_ccmList[CCMID_InRadiation].value=-99999;
  U_ccmList[CCMID_InRadiation].flagStimeRfirst=false;
  }

//////////////////////////////////////////////////Analog
//////////////////////////////////////////////////
double analog1=0;
double analog2=0;

if(setADC1ONOFF&&bootTime>3)
{
analog1=GetAveragedAnalogInputA1();
//5Vモード換算式
analog1=analog1/204.6;
U_ccmList[CCMID_Volt1].value=(long)(analog1*1000);

}
else
{
  analog1=-9.999;
  U_ccmList[CCMID_Volt1].value=-9999;
  U_ccmList[CCMID_Volt1].flagStimeRfirst=false;
  }

if(setADC2ONOFF&&bootTime>3)
{
analog2=GetAveragedAnalogInputA2();
//5Vモード換算式
analog2=analog2/204.6;
U_ccmList[CCMID_Volt2].value=(long)(analog2*1000);
}
else
{
  analog2=-9.999;
  U_ccmList[CCMID_Volt2].value=-9999;
  U_ccmList[CCMID_Volt2].flagStimeRfirst=false;
  }


//////////////////////////////////////////////////温度センサA3,A4
//////////////////////////////////////////////////
   if(setA3ONOFF)
        {
          U_ccmList[CCMID_ExTemp1].value=(long)(ExTempSens.autoRead1Sec(DS18B20_SENSOR1)*10.0);
        }
   else
        {
        ExTempSens.dssts[DS18B20_SENSOR1].measurecount=0;
        ExTempSens.dssts[DS18B20_SENSOR1].errorcount=0;       
        U_ccmList[CCMID_ExTemp1].value=-9999;
        }
   if(U_ccmList[CCMID_ExTemp1].value<-9990){U_ccmList[CCMID_ExTemp1].flagStimeRfirst=false;}


   if(setA4ONOFF)
        {
          U_ccmList[CCMID_ExTemp2].value=(long)(ExTempSens.autoRead1Sec(DS18B20_SENSOR2)*10.0);
        }
   else
        {
        ExTempSens.dssts[DS18B20_SENSOR2].measurecount=0;
        ExTempSens.dssts[DS18B20_SENSOR2].errorcount=0;       
        U_ccmList[CCMID_ExTemp2].value=-9999;
        }
   if(U_ccmList[CCMID_ExTemp2].value<-9990){U_ccmList[CCMID_ExTemp2].flagStimeRfirst=false;}       

//////////////////////////////////////////////////LCD
//////////////////////////////////////////////////

      //CCMの受信状況を調べる
      int recvCCM=0;//受信を必要とするCCMの数
      int recvCount=0;//実際に受信に成功したCCMの数
      int i;
      for(i=0;i<U_MAX_CCM;i++)
      {
        if(!U_ccmList[i].sender)
          {
            recvCCM++;
            if(U_ccmList[i].validity)
                {recvCount++;}
            }
        }

  LCD.clear();
  
  //LCD.newline();

  if(co2CBCount>0)
      {LCD.printCO2Calibration(co2CBCount);}
  else if(!sht31ret && setSHT31ONOFF && bootTime>5)
      {LCD.printSHT31Error();}
  else if(rpm==0 && setFANONOFF && bootTime>5)
      {LCD.printFanStopError();}
  else if(co2==0 && setS300ONOFF && bootTime>5)
      {LCD.printCO2Error();}
  else if(setA3ONOFF && ExTempSens.dssts[DS18B20_SENSOR1].errorcount>3)
      {LCD.printDS18B20Error(DS18B20_SENSOR1);}
  else if(setA4ONOFF && ExTempSens.dssts[DS18B20_SENSOR2].errorcount>3)
      {LCD.printDS18B20Error(DS18B20_SENSOR2);}
  else
      {
        

        if(setSHT31ONOFF)
          {
          LCD.printSensors(sht3x.temp,sht3x.humidity,sht3x.humiddiff,rpm,U_ccmList[CCMID_InAirCO2].value,radiation,
                            analog1,analog2,(double)U_ccmList[CCMID_ExTemp1].value/10.0,(double)U_ccmList[CCMID_ExTemp2].value/10.0
                            ,recvCCM,recvCount);
          }
       else
          {
          LCD.printSensors(-999.9,-99.9,-9.9,rpm,U_ccmList[CCMID_InAirCO2].value,radiation,
                            analog1,analog2,(double)U_ccmList[CCMID_ExTemp1].value/10.0,(double)U_ccmList[CCMID_ExTemp2].value/10.0
                            ,recvCCM,recvCount);
          }
      
      }

    
//////////////////////////////////////////////////警報
//////////////////////////////////////////////////
if(co2CBCount)
  {
   if((millis()/2000)%3==0)
     {tone(PIN_SPK,800,100);
     //SDCard.DebugWrite(AeGPS.Year,AeGPS.Mon,AeGPS.Day,AeGPS.Hour,AeGPS.Min,AeGPS.Sec,101);
     }
   U_ccmList[CCMID_cnd].value|=CO2_CALIBLATION;
  }
else if(!sht31ret && setSHT31ONOFF && bootTime>5)
  {
    tone(PIN_SPK,800,200);
    //SDCard.DebugWrite(AeGPS.Year,AeGPS.Mon,AeGPS.Day,AeGPS.Hour,AeGPS.Min,AeGPS.Sec,102);
    U_ccmList[CCMID_cnd].value|=OPRMODE_ERR_SHT3xSENSERR;
  }
else if(fanErrorCount>5 && setFANONOFF && bootTime>5)
  {
    tone(PIN_SPK,800,300);
    //SDCard.DebugWrite(AeGPS.Year,AeGPS.Mon,AeGPS.Day,AeGPS.Hour,AeGPS.Min,AeGPS.Sec,103);
    U_ccmList[CCMID_cnd].value|=OPRMODE_ERR_FANSTOP;
   }
else if(co2==0 && setS300ONOFF && bootTime>5)
  {
    tone(PIN_SPK,800,200);
    //SDCard.DebugWrite(AeGPS.Year,AeGPS.Mon,AeGPS.Day,AeGPS.Hour,AeGPS.Min,AeGPS.Sec,104);
    U_ccmList[CCMID_cnd].value|=OPRMODE_ERR_S300CO2;
  }
else if(setA3ONOFF && ExTempSens.dssts[DS18B20_SENSOR1].errorcount>2)
      {
      tone(PIN_SPK,800,200);      
      //SDCard.DebugWrite(AeGPS.Year,AeGPS.Mon,AeGPS.Day,AeGPS.Hour,AeGPS.Min,AeGPS.Sec,105);
      U_ccmList[CCMID_cnd].value|=OPRMODE_ERR_DS18B20;
      }
else if(setA4ONOFF && ExTempSens.dssts[DS18B20_SENSOR2].errorcount>2)
      {
      tone(PIN_SPK,800,200);      
      //SDCard.DebugWrite(AeGPS.Year,AeGPS.Mon,AeGPS.Day,AeGPS.Hour,AeGPS.Min,AeGPS.Sec,106);
      U_ccmList[CCMID_cnd].value|=OPRMODE_ERR_DS18B20;
      }
/*else if(SDCard.SDSts==SDCARD_ERROR_DENIED && digitalRead(SDCARD_RECSW))
  {
    tone(PIN_SPK,800,400);
    U_ccmList[CCMID_cnd].value|=OPRMODE_ERR_SDERROR;
   }*/
}


void UserEveryMinute(){}


void UserEveryLoop(){
/*
AeGPS.GPSRead();
SDCard.UpdateSdStatus();
statusSD=SDCard.SDSts;
if(!digitalRead(SDCARD_RECSW))
  {statusSD=3;}//SD stop
  */

}
//-----------------------------------------------------------------------
void loop(){
UECSloop();
//Analog Input for Average
AnalogSampling();
//wdt
if((UECSnowmillis/1000)%2){digitalWrite(PIN_WDT,HIGH);}
else{digitalWrite(PIN_WDT,LOW);}
}

//------------------------起動した時に実行される初期化処理
void setup(){
Serial.begin(9600);
pinMode(PIN_WDT,OUTPUT);
pinMode(PIN_FANRPM,INPUT_PULLUP);
digitalWrite(PIN_WDT,!digitalRead(PIN_WDT));//WDTをリセット
tone(PIN_SPK,1000,200);
delay(200);
tone(PIN_SPK,2000,200);

LCD.begin();
digitalWrite(PIN_WDT,!digitalRead(PIN_WDT));//WDTをリセット
UECSsetup();
setEXESELECT=0;//CO2校正モードは起動時には常にOFF
OnWebFormRecieved();//Web用ユーザーインターフェースの初期化
LCD.printMyIP();

digitalWrite(PIN_WDT,!digitalRead(PIN_WDT));//WDTをリセット
delay(2000);
//AeGPS.begin();
digitalWrite(PIN_WDT,!digitalRead(PIN_WDT));//WDTをリセット
//SDCard.begin();
digitalWrite(PIN_WDT,!digitalRead(PIN_WDT));//WDTをリセット
if(!sht3x.begin(0x45))
  {sht3x.begin(0x44);}
digitalWrite(PIN_WDT,!digitalRead(PIN_WDT));//WDTをリセット
CO2S300.begin(S300I2C_ADDR);
digitalWrite(PIN_WDT,!digitalRead(PIN_WDT));//WDTをリセット

}

//---------------------パルス幅から回転数の計算
double CountFanRPM(){
//PCファンは1回転で2パルス出る
//LOW-HIGH間の時間をマイクロ秒単位で2回測って合計
bool fansts;
bool nowsts;
unsigned long stime;
unsigned long etime;
double timediff=0;

for(int i=0;i<2;i++)
  {
  etime=micros();
  while(1)//頭出し1
    {
    if(digitalRead(PIN_FANRPM)==LOW){break;}
    if(micros()-etime>10000 ||micros()<etime){return 0.0;}
    }
  
  stime=micros();
  while(1)//頭出し2
    {
    if(digitalRead(PIN_FANRPM)==HIGH){break;} 
    if(micros()-stime>10000 ||micros()<stime){return 0.0;}
    }
  
  etime=micros();//計測
  while(1)
    {
    if(digitalRead(PIN_FANRPM)==LOW){break;}
    if(micros()-etime>10000 ||micros()<etime){return 0.0;}
    }
  
  stime=micros();//計測
  while(1)
    {
    if(digitalRead(PIN_FANRPM)==HIGH){break;} 
    if(micros()-stime>10000 ||micros()<stime){return 0.0;}
    }
  timediff+=micros()-etime;
  }
if(timediff==0){return 0.0;}
//最後に１分をマイクロ秒に変換した値を割る
return 60000000.0/timediff;
}
//--------------------------------------------
//アナログ入力の平均化用のサンプリング
//----------------------------------------------
#define ANALOG_AVERAGE_TIMES  100//アナログ入力の平均化数
int AnalogCH0rec[ANALOG_AVERAGE_TIMES];
int AnalogCH1rec[ANALOG_AVERAGE_TIMES];
int AnalogCH2rec[ANALOG_AVERAGE_TIMES];
int AnalogAverageCount=0;
void AnalogSampling(void)
{
  AnalogCH0rec[AnalogAverageCount]=analogRead(A0);
  AnalogCH1rec[AnalogAverageCount]=analogRead(A1);
  AnalogCH2rec[AnalogAverageCount]=analogRead(A2);
AnalogAverageCount++;
AnalogAverageCount%=ANALOG_AVERAGE_TIMES;
}
//----------------------------------------------
//アナログ入力の平均化
//----------------------------------------------
double GetAveragedAnalogInputA0()
{
double ret=0;
for(int i=0;i<ANALOG_AVERAGE_TIMES;i++)
  {ret+=AnalogCH0rec[AnalogAverageCount];}
 return ret/ANALOG_AVERAGE_TIMES;
}
//----------------------------------------------
double GetAveragedAnalogInputA1()
{
double ret=0;
for(int i=0;i<ANALOG_AVERAGE_TIMES;i++)
  {ret+=AnalogCH1rec[AnalogAverageCount];}
 return ret/ANALOG_AVERAGE_TIMES;
}
//----------------------------------------------
double GetAveragedAnalogInputA2()
{
double ret=0;
for(int i=0;i<ANALOG_AVERAGE_TIMES;i++)
  {ret+=AnalogCH2rec[AnalogAverageCount];}
 return ret/ANALOG_AVERAGE_TIMES;
}
