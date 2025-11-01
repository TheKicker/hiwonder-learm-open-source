#include <Wire.h>
#include "IIC.hpp"


void IIC::init(uint8_t sda, uint8_t scl)
{
  Wire.setPins(sda, scl);
  Wire.begin();
  // Wire.setClock(100000); // è®¾ç½®ä¸º 100kHz
}

//åå­è
bool IIC::wireWriteByte(uint8_t addr, uint8_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(val);
    if( Wire.endTransmission() != 0 )
    {
        return false;
    }
    return true;
}

//åå¤ä¸ªå­èï¼ä¸ç¨å¯å­å¨ï¼
bool IIC::wireWritemultiByte(uint8_t addr, uint8_t *val, unsigned int len)
{
    unsigned char i = 0;
    Wire.beginTransmission(addr);
    for(i = 0; i < len; i++) 
    {
        Wire.write(val[i]);
    }
    if( Wire.endTransmission() != 0 ) 
    {
        return false;
    }
    return true;
}

//è¯»æå®é¿åº¦å­èï¼ä¸ç¨å¯å­å¨ï¼
int IIC::wireReadmultiByte(uint8_t addr, uint8_t *val, unsigned int len)
{
    unsigned char i = 0;
    Wire.requestFrom(addr, len);
    while (Wire.available())
    {
        if (i >= len) 
        {
            return -1;
        }
        val[i] = Wire.read();
        i++;
    }
    /* Read block data */    
    return i;
}


//åå¤ä¸ªå­è
bool IIC::wireWriteDataArray(uint8_t addr, uint8_t reg,uint8_t *val,unsigned int len)
{
    unsigned int i;

    Wire.beginTransmission(addr);
    Wire.write(reg);
    for(i = 0; i < len; i++) 
    {
        Wire.write(val[i]);
    }
    if( Wire.endTransmission() != 0 ) 
    {
        return false;
    }
    return true;
}

//è¯»æå®é¿åº¦å­è
int IIC::wireReadDataArray(uint8_t addr, uint8_t reg, uint8_t *val, unsigned int len)
{
    unsigned char i = 0;  
    /* Indicate which register we want to read from */
    if (!wireWriteByte(addr, reg)) 
    {
        return -1;
    }
    Wire.requestFrom(addr, len);
    while (Wire.available()) 
    {
        if (i >= len) 
        {
            return -1;
        }
        val[i] = Wire.read();
        i++;
    }
    /* Read block data */    
    return i;
}
