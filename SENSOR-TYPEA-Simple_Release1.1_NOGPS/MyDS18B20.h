//MyDS18B20 Library
//DS18B20 1wire温度センサ用ライブラリ
//Arduino用
//開発：UECS研究会
//Ver1.0 2021/10/6
//このライブラリはArduno標準ライブラリとOneWireライブラリを使用している
//利用にあたってはOneWireライブラリ(https://github.com/PaulStoffregen/OneWire)をインストールすること
/*
ライセンス
CC BY 4.0
https://creativecommons.org/licenses/by/4.0/
You are free to:
Share
	copy and redistribute the material in any medium or format
Adapt
	remix, transform, and build upon the material
	for any purpose, even commercially.
	This license is acceptable for Free Cultural Works.
	The licensor cannot revoke these freedoms as long as you follow the license terms.

Under the following terms:
Attribution 
	You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
	No additional restrictions ? You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.

Notices:
	You do not have to comply with the license for elements of the material in the public domain or where your use is permitted by an applicable exception or limitation.
	No warranties are given. The license may not give you all of the permissions necessary for your intended use. For example, other rights such as publicity, privacy, or moral rights may limit how you use the material.
*/


#ifndef _MYDS18B20_H_
#define _MYDS18B20_H_

#include "Arduino.h"
#include <OneWire.h>

#define DS18B20_MAXSENSORS    2
#define DS18B20_SENSOR1   0
#define DS18B20_SENSOR2   1


//センサの状態を保持する構造体
struct ds18sts{
  byte addr[8]; //センサ固有アドレス
  byte type;    //センサ分解能
  bool enabled; //接続確認済みフラグ
  byte measurecount;//連続計測成功数(max10)
  byte errorcount;//連続計測失敗数(max10)
  double TempTemp;//直前の値を保存
  char count;//コマンドの与えた状態を保存
};

class DS18B20 {
  public:
    ds18sts dssts[DS18B20_MAXSENSORS];
    double autoRead1Sec(char ch);
    private:
    OneWire ds[DS18B20_MAXSENSORS]={A3,A4};//センサ用のクラスをピンごとに複数生成する
    void OneWireSendStart(char ch);
    bool OneWireSearch(char ch);
    double OneWireGetTemp(char ch);    
};

#endif
