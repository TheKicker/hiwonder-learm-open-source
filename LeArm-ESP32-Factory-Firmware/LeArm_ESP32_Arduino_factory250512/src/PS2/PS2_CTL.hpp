#ifndef __PS2_PORTING_H__
#define __PS2_PORTING_H__
#include "./../../Robot_arm.hpp"
#include "./../../Hiwonder.hpp"
/*
*	START：位置复位
*	MODE：单舵机控制模式/坐标控制模式/动作组模式
* 
*	-------------单舵机控制模式-------------
*	左摇杆控制爪子舵机转角，右摇杆控制爪子舵机张闭
*	LEFT：云台左转	RIGHT1：云台右转
*	UP:1号舵机向前	DOWN：1号舵机向后
*	L1:2号舵机向前	L2:2号舵机向后
*	R1:3号舵机向前	R2:3号舵机向后
* □：爪子左旋 ○：爪子右旋
*	△：爪子闭合 ×：爪子张开		
* LEFTPRESS：减速	RIGHTPRESS：加速	(到达极限值 蜂鸣器滴滴三声) 
*	-------------单舵机控制模式-------------
*	-------------动作组模式-------------
*/
#include "stdint.h"

#define FRAME_HEADER						0x55
#define PS2_PACKET_LENGTH					0x0A
// #define PS2_FRAME_LENGTH					  12
#define MAX_PS2_RB_BUFFER_LENGTH			  	  64

typedef enum
{
	PS2_SINGLE_SERVO_MODE = 1,
	PS2_COORDINATE_MODE
}PS2ModeStatusTypeDef;

typedef enum
{
	UNPACK_FINISH = 0,
	UNPACK_START
}PackStatusTypeDef;

#pragma pack()
typedef struct
{
	uint8_t mode;

	uint8_t left_joystick_x;
	uint8_t left_joystick_y;
	uint8_t right_joystick_x;
	uint8_t right_joystick_y;
	
	uint8_t bit_up : 						1;
	uint8_t bit_down : 						1;
	uint8_t bit_left : 						1;
	uint8_t bit_right : 					1;
	
	uint8_t bit_start : 				1;
	uint8_t bit_select : 				1;
	
	uint8_t bit_cross : 				1;
	uint8_t bit_circle : 				1;
	uint8_t bit_square : 				1;
	uint8_t bit_triangle : 				1;
	
	uint8_t bit_l1 : 					1;
	uint8_t bit_l2 : 					1;
	uint8_t bit_r1 : 					1;
	uint8_t bit_r2 : 					1;

	uint8_t bit_leftjoystick_press : 	1;
	uint8_t bit_rightjoystick_press : 	1;

}PS2KeyValueObjectTypeDef;


class PS2_CTL
{
    public:
        void init(void);
        void clear_rec(void);
        int get_result(LeArm_t* robot,Led_t* led,Buzzer_t* buzzer);
        void PS2_Task(LeArm_t* robot,Led_t* led,Buzzer_t* buzzer);
    private:
        bool rec_flag;
        uint8_t mode_count;
        uint8_t mode_status;
        uint8_t reset_status;
        uint8_t unpack_status;
        uint32_t action_running_time;
        PS2KeyValueObjectTypeDef keyvalue;
		PS2KeyValueObjectTypeDef last_keyvalue;
		uint8_t recbuff[20];
        void receive_msg(void);
};


#endif
