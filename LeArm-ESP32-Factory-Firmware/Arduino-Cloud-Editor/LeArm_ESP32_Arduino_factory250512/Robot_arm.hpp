#ifndef __ROBOT_ARM_H__
#define __ROBOT_ARM_H__
/**
 * @file robot_arm.h
 * @author Min
 * @brief æºæ¢°èæ§å¶å®ç°
 * @version 1.0
 * @date 2024-12-31
 *
 * @copyright Copyright (c) 2024 Hiwonder
 *
 */
#include "stdint.h"
#include "stdbool.h"
#include "Hiwonder.hpp"
#include "Kinematics.hpp"
#include "Config.h"

#define SERVO_NUM               6

// çæ¬å·
#define SOFTWARE_VERSION        1

#define DEFAULT_X						  0.0f
#define DEFAULT_Y						  0.0f
#define DEFAULT_Z						  0.0f
#define DEFAULT_CLAW_OPEN_ANGLE			 90.0f
#define DEFAULT_CLAW_ROTATION_ANGLE		 90.0f

#define MIN_OPEN_ANGLE					  0.0f
#define MAX_OPEN_ANGLE					 90.0f
#define MIN_ROTATION_ANGLE			 	-90.0f
#define MAX_ROTATION_ANGLE				 90.0f

#define MIN_PITCH						-90.0f
#define MAX_PITCH						 90.0f

/* æ°æ®å­æ¾å¨Flashä¸­çèµ·å§åºå°å */
#define LOGO_BASE_ADDRESS          		   0UL  /* è¯¥åºå°åç¨äºå­æ¾æ è¯LOGO */
#define SERVOS_OFFSET_BASE_ADDRESS				 4096UL	/* èµæºåå·®ä¿å­çåºå°å */
#define ACTION_FRAME_SUM_BASE_ADDRESS 	 8192UL  //4096UL  /* è¯¥åºå°åç¨äºå­æ¾æ¯ä¸ªå¨ä½ç»æå¤å°å¨ä½ */
#define ACTION_GROUP_BASE_ADDRESS 		 12288UL //8192UL	/* è¯¥åºå°åç¨äºå­æ¾ä¸è½½çå¨ä½ç»æä»¶ */

#define ACTION_FRAME_SIZE					21  /* ä¸ä¸ªå¨ä½å¸§å 32ä¸ªå­è */ 
#define ACTION_GROUP_SIZE			 	  8192  /* 1ä¸ªå¨ä½ç»ç8KBåå­ç©ºé´ */
#define ACTION_GROUP_MAX_NUM               255  /* é»è®¤æå¤å­æ¾255ä¸ªå¨ä½ç» */

/* è·å¾Açä½å«ä½ */
#define GET_LOW_BYTE(A) ((uint8_t)(A))
/* è·å¾Açé«å«ä½ */
#define GET_HIGH_BYTE(A) ((uint8_t)((A) >> 8))
/* å°é«ä½å«ä½åæä¸ºåå­ä½ */
#define MERGE_HL(A, B) ((((uint16_t)(A)) << 8) | (uint8_t)(B))

#define DEFAULT_X						 15.0f
#define DEFAULT_Y						  0.0f
#define DEFAULT_Z						  2.0f

typedef enum
{
	ACTION_FRAME_START = 0,
	ACTION_FRAME_RUNNING,
	ACTION_FRAME_IDLE
}ActionFrameStatusTypeDef;

typedef enum
{
	ACTION_GROUP_START = 0,
	ACTION_GROUP_RUNNING,
	ACTION_GROUP_END_PERIOD,
	ACTION_GROUP_IDLE
}ActionGroupStatusTypeDef;

typedef struct
{
	uint8_t 				 index;				/* å½åå¨ä½å¸§ç¼å· */
	uint32_t 				 time;				/* å½åå¨ä½å¸§çè¿è¡æ¶é´ */
	uint8_t					 status;			/* å½åè¿è¡æ å¿ */
}ActionFrameHandleTypeDef;

typedef struct
{
	uint8_t 				 index;				/* å½åå¨ä½ç»çç¼å· */
	uint8_t					 sum;				/* å½åå¨ä½ç»çå¨ä½å¸§æ»æ° */
	uint8_t					 running_times;		/* å½åå¨ä½ç»çè¿è¡æ¬¡æ° */
	uint8_t					 status;			/* å½åè¿è¡æ å¿ */
	uint32_t				 time;				/* å½åå¨ä½ç»çè¿è¡æ¶é´ */

	ActionFrameHandleTypeDef frame;
	
}ActionGroupHandleTypeDef;

typedef struct
{
	uint8_t 				 cmd;
	ActionGroupHandleTypeDef action_group;
}RobotArmHandleTypeDef;



class LeArm_t{
    public:
        void init(void);
        void reset(uint32_t time = 800);
        /**
        * @brief æºæ¢°èåæ æ§å¶æ¥å£
        * 
        * @param  target_x 	ç®æ xè½´åæ 
        * @param  target_y		ç®æ yè½´åæ 
        * @param  target_z		ç®æ zè½´åæ 
        * @param  pitch		ç®æ ä¿¯ä»°è§
        * @param  min_pitch	æå°ä¿¯ä»°è§
        * @param  max_pitch	æå¤§ä¿¯ä»°è§
        * @param  time			è¿è¡æ¶é´
        * @return true			æè§£
        * 		   false		æ è§£
        */
        uint8_t coordinate_set(float target_x,float target_y,float target_z,float pitch,float min_pitch,float max_pitch,uint32_t time);

        /* åå³èæ§å¶æ¥å£ */
        /* angleèå´[ 0 , 90 ] */
        void claw_set(float open_angle, uint32_t open_angle_time);
        /* angleèå´[ -90 , 90 ] */
        void roll_set(float rotation_angle, uint32_t rotation_angle_time);
        /* idèå´[ 1 , 6 ]ä»ä¸å°ä¸é¡ºåº */
        void knot_run(uint8_t id, int target_duty, uint32_t time);
        /* idèå´[ 1 , 6 ]ä»ä¸å°ä¸é¡ºåº */
        void knot_stop(uint8_t id);
        /* èµæºä½ç½®è¯»åï¼ä»¥PWMèµæºä¸ºåï¼ */
        uint16_t knot_read(uint8_t id);

// #if (SERVO_TYPE == TYPE_PWM_SERVO)
        /* å¤æ­èµæºæ¯å¦è¿å¨å®æï¼æ»çº¿èµæºæ æ³ä½¿ç¨è¯¥å½æ°ï¼ */
        bool knot_finish(uint8_t id);
// #else
        void serial_servo_offset_init(void);
// #endif

        /* å¨ä½ç»æ§å¶æ¥å£ */
        void action_group_reset(void);
        void action_group_stop(void);
        void action_group_erase(void);
        bool action_group_run(uint8_t action_group_index, uint8_t repeat_times);

        void action_run(uint8_t action_group_index, uint8_t repeat_times);

        /**
        * @brief å¨ä½ç»æ°æ®åå¥æ¥å£
        * 
        * @param  self
        * @param  action_group_index 	å¨ä½ç»ç¼å·
        * @param  frame_num			è¯¥å¨ä½ç»çå¨ä½å¸§æ»æ°
        * @param  frame_index			åå¥çå¨ä½å¸§æ¯ç¬¬å å¸§ï¼åå¼èå´[0,255]
        * @param  pdata				å¸§æ°æ®æé
        * @param  size					å¸§æ°æ®é¿åº¦
        */
        int action_group_save(uint8_t action_group_number, uint8_t frame_num,uint8_t frame_index,uint8_t* pdata,uint16_t size);

        /* åå·®è®¾ç½®æ¥å£ */
        int8_t offset_read(uint8_t id);
        void offset_set(uint8_t id, int8_t value);
        void offset_save(void);


        uint8_t get_servo_type(void);
        
    private:
        Flash_ctl_t flash_obj;
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
        PwmServo_t  pwmservo_obj;
// #else
        BusServo_t  busservo_obj;
        int8_t      bus_servo_offset[6];
// #endif
        KinematicsObjectTypeDef  kinematics;
        RobotArmHandleTypeDef robot_arm;

        uint8_t servo_type;
        uint8_t read_servo_type(void);
        uint8_t action_frame_run(uint8_t action_group_index, uint8_t frame_index);
        void theta2servo(KinematicsObjectTypeDef* self, float time);
        void action_group_init();
};

#endif
