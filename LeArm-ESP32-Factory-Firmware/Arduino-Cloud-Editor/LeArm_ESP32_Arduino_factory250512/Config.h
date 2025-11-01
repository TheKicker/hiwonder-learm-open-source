#ifndef __CONFIG_H 
#define __CONFIG_H 

#define SERVO_NUM 6

#define TYPE_PWM_SERVO		1
#define TYPE_SERIAL_SERVO	2
/* æºæ¢°èèµæºç±»åéæ© */
#define SERVO_TYPE			TYPE_PWM_SERVO

// arm.coordinate_set(15,0,2,0,-90,90,1000);
#if (SERVO_TYPE == TYPE_PWM_SERVO)
#define SERVO1_RESET_DUTY 				   770
#define SERVO2_RESET_DUTY 				  1500
#define SERVO3_RESET_DUTY 				   644
#define SERVO4_RESET_DUTY 				   511
#define SERVO5_RESET_DUTY 				  1255
#define SERVO6_RESET_DUTY 				  1500
// #define MAX_DUTY 2500
// #define MIN_DUTY 500
#else
#define SERVO1_RESET_DUTY                         226
#define SERVO2_RESET_DUTY                         500 
#define SERVO3_RESET_DUTY                         177 
#define SERVO4_RESET_DUTY                         129 
#define SERVO5_RESET_DUTY                         408 
#define SERVO6_RESET_DUTY                         500
// #define MAX_DUTY 875
// #define MIN_DUTY 125
#endif

// uint16_t SERVO1_RESET_DUTY = 770;
// uint16_t SERVO2_RESET_DUTY = 1500;
// uint16_t SERVO3_RESET_DUTY = 644;
// uint16_t SERVO4_RESET_DUTY = 511;
// uint16_t SERVO5_RESET_DUTY = 1255;
// uint16_t SERVO6_RESET_DUTY = 1500;
// uint16_t MAX_DUTY = 2500;
// uint16_t MIN_DUTY = 500;

#define SERVO_1   19
#define SERVO_2   18
#define SERVO_3   5
#define SERVO_4   4
#define SERVO_5   0
#define SERVO_6   15

#define IIC_SDA     17
#define IIC_SCL     16

#define BLE_TX      1
#define BLE_RX      3

#define BUS_TX      12
#define BUS_RX      35
#define BUS_EN      14

#define ADC_BAT     39

#define IO_BUTTON    36

#define IO_LED      26

#define IO_BUZZER   27

#define USB_TX      34

#define FLASH_CLK   23
#define FLASH_DI    22
#define FLASH_DO    21
#define FLASH_CS    2

#define IO_BLE_CTL  25

/* 5V GND IO2 IO13 */
#define IO13       13

/* 5V GND PA5 PA4 */
#define PA5         33
#define PA4         32

#endif //__CONFIG_H 
