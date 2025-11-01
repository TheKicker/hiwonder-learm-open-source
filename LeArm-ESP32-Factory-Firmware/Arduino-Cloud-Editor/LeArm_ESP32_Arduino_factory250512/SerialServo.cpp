#include "Hiwonder.hpp"
#include "stdint.h"
#include "Config.h"

#define READ_LEVEL  LOW
#define WRITE_LEVEL HIGH

//æ ¡éªå
uint8_t BusServo_t::CheckSum(uint8_t buf[])
{
  uint8_t i;
  uint16_t temp = 0;
  for (i = 2; i < buf[3] + 2; i++) {
    temp += buf[i];
  }
  temp = ~temp;
  i = (uint8_t)temp;
  return i;
}

void BusServo_t::init(HardwareSerial* Serial_obj)
{
  SerialX = Serial_obj;
  pinMode(BUS_EN, OUTPUT);
  digitalWrite(BUS_EN,WRITE_LEVEL);
}

//è§£ææ¥æ¶å°çæ°æ®åä¿¡æ¯ï¼å¹¶è¿å
int BusServo_t::ReceiveHandle(uint8_t *ret)
{
  bool frameStarted = false;
  bool receiveFinished = false;
  uint8_t frameCount = 0;
  uint8_t dataCount = 0;
  uint8_t dataLength = 2;
  uint8_t rxBuf;
  uint8_t recvBuf[32];
  uint8_t i;
  while (SerialX->available()) {
    rxBuf = SerialX->read();
    delayMicroseconds(100);
    if (!frameStarted) {
      if (rxBuf == LOBOT_SERVO_FRAME_HEADER) {
        frameCount++;
        if (frameCount == 2) {
          frameCount = 0;
          frameStarted = true;
          dataCount = 1;
        }
      }
      else {
        frameStarted = false;
        dataCount = 0;
        frameCount = 0;
      }
    }
    if (frameStarted) {
      recvBuf[dataCount] = (uint8_t)rxBuf;
      if (dataCount == 3) {
        dataLength = recvBuf[dataCount];
        if (dataLength < 3 || dataCount > 7) {
          dataLength = 2;
          frameStarted = false;
        }
      }
      dataCount++;
      if (dataCount == dataLength + 3) {

        if (CheckSum(recvBuf) == recvBuf[dataCount - 1]) {

          frameStarted = false;
          memcpy(ret, recvBuf + 4, dataLength);
          return 1;
        }
        return -1;
      }
    }
  }
  return 0;
}
//åå¥èµæºID
void BusServo_t::SetID(uint8_t oldID, uint8_t newID)
{
  uint8_t buf[7];
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = oldID;
  buf[3] = 4;
  buf[4] = LOBOT_SERVO_ID_WRITE;
  buf[5] = newID;
  buf[6] = CheckSum(buf);
  SerialX->write(buf, 7);
}

//æ§å¶èµæºè½¬å¨
void BusServo_t::set_position(uint8_t servo_ID, int16_t position, uint16_t duration)
{
  uint8_t buf[10];
  if(position < 0)
    position = 0;
  if(position > 1000)
    position = 1000;
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = servo_ID;
  buf[3] = 7;
  buf[4] = LOBOT_SERVO_MOVE_TIME_WRITE;
  buf[5] = GET_LOW_BYTE(position);
  buf[6] = GET_HIGH_BYTE(position);
  buf[7] = GET_LOW_BYTE(duration);
  buf[8] = GET_HIGH_BYTE(duration);
  buf[9] = CheckSum(buf);
  SerialX->write(buf, 10);
}

void BusServo_t::set_angle(uint16_t servo_ID, uint32_t angle, uint16_t duration)
{
  uint8_t buf[10];
  uint16_t position = angle > 240 ? 1000 : (angle*1000/240);
  if(position < 0)
    position = 0;
  if(position > 1000)
    position = 1000;
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = servo_ID;
  buf[3] = 7;
  buf[4] = LOBOT_SERVO_MOVE_TIME_WRITE;
  buf[5] = GET_LOW_BYTE(position);
  buf[6] = GET_HIGH_BYTE(position);
  buf[7] = GET_LOW_BYTE(duration);
  buf[8] = GET_HIGH_BYTE(duration);
  buf[9] = CheckSum(buf);
  SerialX->write(buf, 10);
}

//è¯»åID
int BusServo_t::ReadID(void)
{
  int count = 100; //10000
  int ret;
  uint8_t buf[6];
  
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = ID_ALL;  //ID_ALLä¸º254ï¼è¡¨ç¤ºåææèµæºè¿è¡å¹¿æ­ï¼å¯ç¨äºè¯»åæªç¥IDçèµæºä¿¡æ¯
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_ID_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }
  
  if (ReceiveHandle(buf) > 0)
    ret = (int16_t)BYTE_TO_HW(0x00, buf[1]);
  else
    ret = -1024;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è¯»åèµæºä½ç½®
int BusServo_t::read_position(uint8_t servo_ID)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = servo_ID;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_POS_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
    ret = (int16_t)BYTE_TO_HW(buf[2], buf[1]);
  else
    ret = -2048;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

int BusServo_t::read_angle(uint8_t servo_ID)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = servo_ID;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_POS_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
  {
    ret = (int16_t)BYTE_TO_HW(buf[2], buf[1]);
    ret = ret * 240 / 1000;
  }else
    ret = -2048;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è¯»ååå·®
int BusServo_t::ReadDev(uint8_t id)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_ANGLE_OFFSET_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0){
    ret = (int8_t)buf[1];
  }else{
    ret = -1024;
  }
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

// è®¾ç½®åå·®
void BusServo_t::SetDev(uint8_t id, int8_t dev)
{
  uint8_t buf[7];
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 4;
  buf[4] = LOBOT_SERVO_ANGLE_OFFSET_ADJUST;
  buf[5] = (uint8_t)dev;
  buf[6] = CheckSum(buf);

  SerialX->write(buf, 7);
}


// ä¿å­åå·®
void BusServo_t::SaveDev(uint8_t id)
{
  uint8_t buf[6];
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_ANGLE_OFFSET_WRITE;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
}


//è¯»åè½¬å¨èå´
int retL;
int retH;
int BusServo_t::ReadAngleRange(uint8_t id)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_ANGLE_LIMIT_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
    {
      retL = (int16_t)BYTE_TO_HW(buf[2], buf[1]); 
      retH = (int16_t)BYTE_TO_HW(buf[4], buf[3]);
    }
  else
    ret = -1024;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è¯»åçµå
int BusServo_t::ReadVin(uint8_t id)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_VIN_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
    ret = (int16_t)BYTE_TO_HW(buf[2], buf[1]);
  else
    ret = -2049;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è¯»åçµåèå´
int vinL;
int vinH;
int BusServo_t::ReadVinLimit(uint8_t id)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_VIN_LIMIT_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
    {
      vinL = (int16_t)BYTE_TO_HW(buf[2], buf[1]); 
      vinH = (int16_t)BYTE_TO_HW(buf[4], buf[3]);
    }
  else
    ret = -1024;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è¯»åæ¸©åº¦æ¥è­¦éå¼
int BusServo_t::ReadTempLimit(uint8_t id)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_TEMP_MAX_LIMIT_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
    ret = (int16_t)BYTE_TO_HW(0x00, buf[1]);
  else
    ret = -2049;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è¯»åæ¸©åº¦
int BusServo_t::ReadTemp(uint8_t id)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_TEMP_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
    ret = (int16_t)BYTE_TO_HW(0x00, buf[1]);
  else
    ret = -2049;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è¯»åèµæºç¶æ
int BusServo_t::ReadLoadOrUnload(uint8_t id)
{
  int count = 10000;
  int ret;
  uint8_t buf[6];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_LOAD_OR_UNLOAD_READ;
  buf[5] = CheckSum(buf);
  SerialX->write(buf, 6);
  delayMicroseconds(500);
  digitalWrite(BUS_EN,READ_LEVEL);
  delay(5);
  
  while (!SerialX->available()) {
    count -= 1;
    if (count < 0)
    {
      digitalWrite(BUS_EN,WRITE_LEVEL);
      return -2048;
    }
    delay(5);
  }

  if (ReceiveHandle(buf) > 0)
    ret = (int16_t)BYTE_TO_HW(0x00, buf[1]);
  else
    ret = -2049;
  digitalWrite(BUS_EN,WRITE_LEVEL);
  return ret;
}

//è®¾ç½®èµæºæ¨¡å¼
void BusServo_t::SetMode(uint8_t id, uint8_t Mode, int16_t Speed)
{
  uint8_t buf[10];

  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 7;
  buf[4] = LOBOT_SERVO_OR_MOTOR_MODE_WRITE;
  buf[5] = Mode;
  buf[6] = 0;
  buf[7] = GET_LOW_BYTE((uint16_t)Speed);
  buf[8] = GET_HIGH_BYTE((uint16_t)Speed);
  buf[9] = CheckSum(buf);

  SerialX->write(buf, 10);
}

//èµæºä¸çµ
void BusServo_t::Load(uint8_t id)
{
  uint8_t buf[7];
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 4;
  buf[4] = LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE;
  buf[5] = 1;
  buf[6] = CheckSum(buf);
  
  SerialX->write(buf, 7);

}
//èµæºæçµ
void BusServo_t::Unload(uint8_t id)
{
  uint8_t buf[7];
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 4;
  buf[4] = LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE;
  buf[5] = 0;
  buf[6] = CheckSum(buf);
  
  SerialX->write(buf, 7);
}
//åæ­¢è½¬å¨
void BusServo_t::stop(uint8_t servo_ID)
{
  uint8_t buf[6];
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = servo_ID;
  buf[3] = 3;
  buf[4] = LOBOT_SERVO_MOVE_STOP;
  buf[5] = CheckSum(buf);
  
  SerialX->write(buf, 6);
}
