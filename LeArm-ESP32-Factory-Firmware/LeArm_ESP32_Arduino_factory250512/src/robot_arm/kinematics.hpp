#ifndef __KINEMATICS_H__
#define __KINEMATICS_H__

#include "stdbool.h"
#include "stdint.h"

/**
 * @file kinematics.hpp
 * @author Mobius
 * @brief 正逆运动学解算
 * @version 1.0
 * @date 2024-12-13
 *
 * @copyright Copyright (c) 2024 Hiwonder
 *
 */
 
 #define PI 3.1415926f
 
/* 连杆序号按照从底部向上排序 单位：cm*/
#define LINKAGE_1    				 2.89f
#define LINKAGE_2					 10.43f
#define LINKAGE_3		 				8.9f
#define LINKAGE_4	 					17.7f

#define MIN_KNOT6_ANGLE							-90.0f
#define MAX_KNOT6_ANGLE							 90.0f
#define MIN_KNOT5_ANGLE							  0.0f
#define MAX_KNOT5_ANGLE							180.0f
#define MIN_KNOT4_ANGLE							-90.0f
#define MAX_KNOT4_ANGLE							 90.0f
#define MIN_KNOT3_ANGLE							-90.0f
#define MAX_KNOT3_ANGLE							 90.0f


typedef enum 
{
	K_OK = 1,
	INVAILD
}KinematicsStatusTypedef;

typedef struct
{
	float x;
	float y;
	float z;
}VectorObjectTypeDef;

typedef struct
{
	float rad;
	float theta;
}KnotObjectTypeDef;

typedef struct KinematicsObject KinematicsObjectTypeDef;
struct KinematicsObject
{
	float alpha;
	VectorObjectTypeDef vector;
	KnotObjectTypeDef knot[4];
};

/**
 * @brief 弧度制转角度制
 * 
 * @param  rad
 * @return 角度 
 */
float rad2theta(float rad);

/**
 * @brief 角度制转弧度制
 * 
 * @param  theta
 * @return 弧度 
 */
float theta2rad(float theta);

/**
 * @brief 
 * 
 * @param  self		需要控制对象的指针
 * @return NULL 
 */
void kinematics_init(KinematicsObjectTypeDef* self);

/**
 * @brief 逆运动学解算
 * 
 * @param  self		需要控制对象的指针
 * @return  K_OK		有解
 * 			INVAILD	无解
 */
uint8_t ikine(KinematicsObjectTypeDef* self);

/**
 * @brief 正运动学解算
 * 
 * @param  knot0_theta	从下至上第1个关节角度
 * @param  knot1_theta	从下至上第2个关节角度
 * @param  knot2_theta	从下至上第3个关节角度
 * @param  knot3_theta	从下至上第4个关节角度
 * @return VectorObjectTypeDef类型结构体
 */
VectorObjectTypeDef fkine(float knot0_theta, float knot1_theta, float knot2_theta, float knot3_theta);

#endif
