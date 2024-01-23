//Sensirion SHT31温湿度センサ用ライブラリ
//Arduino用
//開発：UECS研究会
//Ver1.0 2021/10/6
//このライブラリはArduno標準ライブラリがあれば単体でも使用できる
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
#ifndef _SHT3x_H_
#define _SHT3x_H_

#include "Arduino.h"
#include "Wire.h"

#define SHT3x_ADDR    0x45

class SHT3x {
  public:
    SHT3x();
    bool autoRead1Sec(); 
    bool begin(unsigned char i2caddr = SHT3x_ADDR);
    bool startMeasure(void);
    bool getTempHumid(void);
    double humidity, temp,humiddiff;
    int retryCount;
  private:
    int writeCommand(unsigned short cmd);
    bool reset(void);
    unsigned char crc8Dallas(const unsigned char *data, int len);
    unsigned char _i2caddr;
    
};

#endif
