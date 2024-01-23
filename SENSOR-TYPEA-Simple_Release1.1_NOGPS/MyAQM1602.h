//AQM1602液晶表示用基本ライブラリ
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

#ifndef MYAQM1602_H
#define MYAQM1602_H

#include <Arduino.h>
#include <Wire.h>
#include "Print.h"

#define AQM1602_I2C_ADDRESS 0x3E 

// commands
#define AQM1602CMD_RETURNHOME 0x02
#define AQM1602CMD_CLEARDISPLAY 0x01
#define AQM1602CMD_DISPLAYCONTROL 0x08
#define AQM1602CMD_DISPLAYON 0x04
#define AQM1602CMD_BLINKON 0x01
#define AQM1602CMD_BLINKOFF 0x00
#define AQM1602CMD_CURSORON 0x02
#define AQM1602CMD_CURSOROFF 0x00

class MyAQM1602 : public Print {
  public:
    MyAQM1602(uint8_t addr = AQM1602_I2C_ADDRESS);
    void command(uint8_t value);
    virtual size_t write(uint8_t value);
    void init(void);
    void begin(void);
    void home(void);
    void clear(void);
    void displayOn(void);
    void newLine(void);
  private:
    uint8_t _displayflags;
    uint8_t _i2caddress;
    void send(uint8_t value, uint8_t mode);
};

#endif
