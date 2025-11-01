#ifndef IIC_H
#define IIC_H

#include <Arduino.h>
#include "Config.h"

class IIC {
  public:
    void init(uint8_t sda = IIC_SDA , uint8_t scl = IIC_SCL);
    bool wireWriteByte(uint8_t addr, uint8_t val);
    bool wireWritemultiByte(uint8_t addr, uint8_t *val, unsigned int len);
    int wireReadmultiByte(uint8_t addr, uint8_t *val, unsigned int len);
    bool wireWriteDataArray(uint8_t addr, uint8_t reg,uint8_t *val,unsigned int len);
    int  wireReadDataArray(uint8_t addr, uint8_t reg, uint8_t *val, unsigned int len);
};

#endif //IIC_H
