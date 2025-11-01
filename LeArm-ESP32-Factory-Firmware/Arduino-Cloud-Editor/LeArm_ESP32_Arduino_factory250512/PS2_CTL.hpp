#ifndef __PS2_PORTING_H__
#define __PS2_PORTING_H__
#include "Robot_arm.hpp"
#include "Hiwonder.hpp"
/*
*	STARTï¼ä½ç½®å¤ä½
*	MODEï¼åèµæºæ§å¶æ¨¡å¼/åæ æ§å¶æ¨¡å¼/å¨ä½ç»æ¨¡å¼
* 
*	-------------åèµæºæ§å¶æ¨¡å¼-------------
*	å·¦æææ§å¶çªå­èµæºè½¬è§ï¼å³æææ§å¶çªå­èµæºå¼ é­
*	LEFTï¼äºå°å·¦è½¬	RIGHT1ï¼äºå°å³è½¬
*	UP:1å·èµæºåå	DOWNï¼1å·èµæºåå
*	L1:2å·èµæºåå	L2:2å·èµæºåå
*	R1:3å·èµæºåå	R2:3å·èµæºåå
* â¡ï¼çªå­å·¦æ âï¼çªå­å³æ
*	â³ï¼çªå­é­å Ãï¼çªå­å¼ å¼		
* LEFTPRESSï¼åé	RIGHTPRESSï¼å é	(å°è¾¾æéå¼ èé¸£å¨æ»´æ»´ä¸å£°) 
*	-------------åèµæºæ§å¶æ¨¡å¼-------------
*	-------------å¨ä½ç»æ¨¡å¼-------------
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
