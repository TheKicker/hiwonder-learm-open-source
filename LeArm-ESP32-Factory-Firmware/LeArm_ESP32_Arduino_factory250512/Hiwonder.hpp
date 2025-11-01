#ifndef __HIWONDER_H
#define __HIWONDER_H

#define ROBOT_LOGO    "LeArm"

//存放起始地址
#define MEM_LOBOT_LOGO_BASE					0L	//"LOBOT"存放基地址，用于识别是否是新FLASH
#define MEM_FRAME_INDEX_SUM_BASE			4096L//每个动作组有多少动作，从这个地址开始存放，共计256个动作组
#define MEM_ACT_FULL_BASE					8192L//动作组文件从这个地址开始存放

//大小
#define ACT_SUB_FRAME_SIZE					64L		//一个动作帧占64字节空间
#define ACT_FULL_SIZE						16384L	//16KB,一套完整动作组占16KB字节

#include "stdint.h"
#include <Ticker.h>
#include <HardwareSerial.h>

typedef enum {
    LED_STAGE_START_NEW_CYCLE,
    LED_STAGE_WATTING_OFF,
    LED_STAGE_WATTING_PERIOD_END,
    LED_STAGE_IDLE,
} LEDStageEnum;

class Led_t{
    public:
        void init(uint8_t pin);
        void on_off(uint8_t state);
        void blink(uint32_t on_time , uint32_t off_time , uint32_t count);

    public:
        uint8_t led_pin;
        Ticker timer_led;
        uint8_t new_flag;
        uint16_t ticks_on;
        uint16_t ticks_off;
        uint16_t repeat;
        LEDStageEnum stage;
        uint32_t ticks_count;
};


typedef enum {
    BUZZER_STAGE_START_NEW_CYCLE,
    BUZZER_STAGE_WATTING_OFF,
    BUZZER_STAGE_WATTING_PERIOD_END,
    BUZZER_STAGE_IDLE,
} BuzzerStageEnum;

#define CHANNEL_DEFAULT 11

class Buzzer_t{
    public:
        void init(uint8_t pin , uint8_t channel = CHANNEL_DEFAULT , uint16_t frequency = 1500);
        void on_off(uint8_t state);
        void blink(uint16_t frequency , uint16_t on_time , uint16_t off_time , uint16_t count);

    public:
        uint8_t buzzer_pin;
        uint8_t buzzer_channel;
        Ticker timer_buzzer;
        uint8_t new_flag;
        uint16_t freq;
        uint16_t ticks_on;
        uint16_t ticks_off;
        uint16_t repeat;
        BuzzerStageEnum stage;
        uint32_t ticks_count;
};

typedef enum {
    BUTTON_STAGE_NORMAL,
    BUTTON_STAGE_PRESS,
    BUTTON_STAGE_LONGPRESS,
} ButtonStageEnum;

typedef enum {
    BUTTON_EVENT_PRESSED = 0x01,           /**< @brief 按钮被按下 */
    BUTTON_EVENT_LONGPRESS = 0x02,         /**< @brief 按钮被长按 */
    BUTTON_EVENT_LONGPRESS_REPEAT = 0x04,  /**< @brief 按钮长按重触发 */
    BUTTON_EVENT_RELEASE_FROM_LP = 0x08,   /**< @brief 按钮从长按中松开 */
    BUTTON_EVENT_RELEASE_FROM_SP = 0x10,   /**< @brief 按钮从短按中松开 */
    BUTTON_EVENT_CLICK = 0x20,             /**< @brief 按钮被点击 */
    BUTTON_EVENT_DOUBLE_CLICK = 0x40,      /**< @brief 按钮被双击 */
    BUTTON_EVENT_TRIPLE_CLICK = 0x80,      /**< @brief 按钮被三连击 */
} ButtonEventIDEnum;

class Button_t{
    public:
        void init(void);
        uint8_t read(uint8_t id);
        void register_callback(void (*function)(uint8_t , ButtonEventIDEnum));

    public:
        // uint8_t button_pin;
        Ticker timer_button;
        void (*event_callback)(uint8_t, ButtonEventIDEnum);
        struct run_st{
          uint32_t last_pin_raw;
          uint32_t last_pin_filtered;
          ButtonStageEnum stage;
          uint32_t ticks_count;
          uint32_t combin_th;
          uint32_t combin_counter;
          uint32_t lp_th;
          uint32_t repeat_th;
        }bt_run[2];
};

class Battery_t{
    public:
        void init(uint8_t pin);
        int read_mV(void);
    private:
        uint8_t battery_pin;
};

class Flash_ctl_t{
    public:
        void init(void);
        void read(uint32_t addr24, uint8_t *buffer, uint32_t size24);
        void write(uint32_t addr24, uint8_t *buffer, uint32_t size24);
        void erase_sector(uint32_t addr24);
        void erase_all(void);
};




/* PWM舵机总控制器 */
class PwmServo_t{
    public:
        void init(void);
        int  set_duty(uint16_t servo_index, uint32_t pulsewidth, uint32_t duration);
        int  set_angle(uint16_t servo_index, uint32_t angle, uint32_t duration);
        
        int  read_duty(uint16_t servo_index);
        int  read_angle(uint16_t servo_index);
        int  set_offset(uint16_t servo_index, int offset);
        int  read_offset(uint16_t servo_index);
        // int  save_offset(void);
        int  stop(uint16_t servo_index);
        // 判断是否到达指定位置
        bool  is_ready(uint16_t servo_index);
        void deinit(void);
};

//宏函数 获得A的低八位
#define GET_LOW_BYTE(A) (uint8_t)((A))
//宏函数 获得A的高八位
#define GET_HIGH_BYTE(A) (uint8_t)((A) >> 8)
//宏函数 以A为高八位 B为低八位 合并为16位整形
#define BYTE_TO_HW(A, B) ((((uint16_t)(A)) << 8) | (uint8_t)(B))
#define ID_ALL 254  //ID为254时，会向所有舵机进行广播，可用于读取未知ID的舵机信息
#define LOBOT_SERVO_FRAME_HEADER         0x55
#define LOBOT_SERVO_MOVE_TIME_WRITE      1
#define LOBOT_SERVO_MOVE_TIME_READ       2
#define LOBOT_SERVO_MOVE_TIME_WAIT_WRITE 7
#define LOBOT_SERVO_MOVE_TIME_WAIT_READ  8
#define LOBOT_SERVO_MOVE_START           11
#define LOBOT_SERVO_MOVE_STOP            12
#define LOBOT_SERVO_ID_WRITE             13
#define LOBOT_SERVO_ID_READ              14
#define LOBOT_SERVO_ANGLE_OFFSET_ADJUST  17
#define LOBOT_SERVO_ANGLE_OFFSET_WRITE   18
#define LOBOT_SERVO_ANGLE_OFFSET_READ    19
#define LOBOT_SERVO_ANGLE_LIMIT_WRITE    20
#define LOBOT_SERVO_ANGLE_LIMIT_READ     21
#define LOBOT_SERVO_VIN_LIMIT_WRITE      22
#define LOBOT_SERVO_VIN_LIMIT_READ       23
#define LOBOT_SERVO_TEMP_MAX_LIMIT_WRITE 24
#define LOBOT_SERVO_TEMP_MAX_LIMIT_READ  25
#define LOBOT_SERVO_TEMP_READ            26
#define LOBOT_SERVO_VIN_READ             27
#define LOBOT_SERVO_POS_READ             28
#define LOBOT_SERVO_OR_MOTOR_MODE_WRITE  29
#define LOBOT_SERVO_OR_MOTOR_MODE_READ   30
#define LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE 31
#define LOBOT_SERVO_LOAD_OR_UNLOAD_READ  32
#define LOBOT_SERVO_LED_CTRL_WRITE       33
#define LOBOT_SERVO_LED_CTRL_READ        34
#define LOBOT_SERVO_LED_ERROR_WRITE      35
#define LOBOT_SERVO_LED_ERROR_READ       36
/* 总线舵机总控制器 */
class BusServo_t{
    public:
        void init(HardwareSerial* Serial_obj);
        void set_position(uint8_t servo_ID, int16_t position, uint16_t duration);
        void set_angle(uint16_t servo_ID, uint32_t angle, uint16_t duration);
        int read_position(uint8_t servo_ID);
        int read_angle(uint8_t servo_ID);

        void SetID(uint8_t oldID, uint8_t newID);
        int ReadID(void);
        void SetDev(uint8_t id, int8_t dev);
        int ReadDev(uint8_t id);
        void SaveDev(uint8_t id);
        int ReadAngleRange(uint8_t id);
        int ReadVin(uint8_t id);
        int ReadVinLimit(uint8_t id);
        int ReadTempLimit(uint8_t id);
        int ReadTemp(uint8_t id);
        int ReadLoadOrUnload(uint8_t id);
        void SetMode(uint8_t id, uint8_t Mode, int16_t Speed);
        void Load(uint8_t id);
        void Unload(uint8_t id);

        void stop(uint8_t servo_ID);

    private:
        HardwareSerial* SerialX;

        uint8_t CheckSum(uint8_t buf[]);
        int ReceiveHandle(uint8_t *ret);
};

#endif //__HIWONDER_H
