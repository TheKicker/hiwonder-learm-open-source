#ifndef __ROBOT_ARM_H__
#define __ROBOT_ARM_H__
/**
 * @file robot_arm.h
 * @author Min
 * @brief 机械臂控制实现
 * @version 1.0
 * @date 2024-12-31
 *
 * @copyright Copyright (c) 2024 Hiwonder
 *
 */
#include "stdint.h"
#include "stdbool.h"
#include "Hiwonder.hpp"
#include "./src/robot_arm/Kinematics.hpp"
#include "Config.h"

#define SERVO_NUM               6

// 版本号
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

/* 数据存放在Flash中的起始基地址 */
#define LOGO_BASE_ADDRESS          		   0UL  /* 该基地址用于存放标识LOGO */
#define SERVOS_OFFSET_BASE_ADDRESS				 4096UL	/* 舵机偏差保存的基地址 */
#define ACTION_FRAME_SUM_BASE_ADDRESS 	 8192UL  //4096UL  /* 该基地址用于存放每个动作组有多少动作 */
#define ACTION_GROUP_BASE_ADDRESS 		 12288UL //8192UL	/* 该基地址用于存放下载的动作组文件 */

#define ACTION_FRAME_SIZE					21  /* 一个动作帧占32个字节 */ 
#define ACTION_GROUP_SIZE			 	  8192  /* 1个动作组留8KB内存空间 */
#define ACTION_GROUP_MAX_NUM               255  /* 默认最多存放255个动作组 */

/* 获得A的低八位 */
#define GET_LOW_BYTE(A) ((uint8_t)(A))
/* 获得A的高八位 */
#define GET_HIGH_BYTE(A) ((uint8_t)((A) >> 8))
/* 将高低八位合成为十六位 */
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
	uint8_t 				 index;				/* 当前动作帧编号 */
	uint32_t 				 time;				/* 当前动作帧的运行时间 */
	uint8_t					 status;			/* 当前运行标志 */
}ActionFrameHandleTypeDef;

typedef struct
{
	uint8_t 				 index;				/* 当前动作组的编号 */
	uint8_t					 sum;				/* 当前动作组的动作帧总数 */
	uint8_t					 running_times;		/* 当前动作组的运行次数 */
	uint8_t					 status;			/* 当前运行标志 */
	uint32_t				 time;				/* 当前动作组的运行时间 */

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
        * @brief 机械臂坐标控制接口
        * 
        * @param  target_x 	目标x轴坐标
        * @param  target_y		目标y轴坐标
        * @param  target_z		目标z轴坐标
        * @param  pitch		目标俯仰角
        * @param  min_pitch	最小俯仰角
        * @param  max_pitch	最大俯仰角
        * @param  time			运行时间
        * @return true			有解
        * 		   false		无解
        */
        uint8_t coordinate_set(float target_x,float target_y,float target_z,float pitch,float min_pitch,float max_pitch,uint32_t time);

        /* 单关节控制接口 */
        /* angle范围[ 0 , 90 ] */
        void claw_set(float open_angle, uint32_t open_angle_time);
        /* angle范围[ -90 , 90 ] */
        void roll_set(float rotation_angle, uint32_t rotation_angle_time);
        /* id范围[ 1 , 6 ]从上到下顺序 */
        void knot_run(uint8_t id, int target_duty, uint32_t time);
        /* id范围[ 1 , 6 ]从上到下顺序 */
        void knot_stop(uint8_t id);
        /* 舵机位置读取（以PWM舵机为准） */
        uint16_t knot_read(uint8_t id);

// #if (SERVO_TYPE == TYPE_PWM_SERVO)
        /* 判断舵机是否运动完成（总线舵机无法使用该函数） */
        bool knot_finish(uint8_t id);
// #else
        void serial_servo_offset_init(void);
// #endif

        /* 动作组控制接口 */
        void action_group_reset(void);
        void action_group_stop(void);
        void action_group_erase(void);
        bool action_group_run(uint8_t action_group_index, uint8_t repeat_times);

        void action_run(uint8_t action_group_index, uint8_t repeat_times);

        /**
        * @brief 动作组数据写入接口
        * 
        * @param  self
        * @param  action_group_index 	动作组编号
        * @param  frame_num			该动作组的动作帧总数
        * @param  frame_index			写入的动作帧是第几帧，取值范围[0,255]
        * @param  pdata				帧数据指针
        * @param  size					帧数据长度
        */
        int action_group_save(uint8_t action_group_number, uint8_t frame_num,uint8_t frame_index,uint8_t* pdata,uint16_t size);

        /* 偏差设置接口 */
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
