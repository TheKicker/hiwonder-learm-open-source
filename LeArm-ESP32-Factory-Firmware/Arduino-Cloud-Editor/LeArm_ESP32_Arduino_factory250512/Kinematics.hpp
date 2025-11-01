#ifndef __KINEMATICS_H__
#define __KINEMATICS_H__

#include "stdbool.h"
#include "stdint.h"

/**
 * @file kinematics.hpp
 * @author Mobius
 * @brief æ­£éè¿å¨å­¦è§£ç®
 * @version 1.0
 * @date 2024-12-13
 *
 * @copyright Copyright (c) 2024 Hiwonder
 *
 */
 
 #define PI 3.1415926f
 
/* è¿æåºå·æç§ä»åºé¨åä¸æåº åä½ï¼cm*/
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
 * @brief å¼§åº¦å¶è½¬è§åº¦å¶
 * 
 * @param  rad
 * @return è§åº¦ 
 */
float rad2theta(float rad);

/**
 * @brief è§åº¦å¶è½¬å¼§åº¦å¶
 * 
 * @param  theta
 * @return å¼§åº¦ 
 */
float theta2rad(float theta);

/**
 * @brief 
 * 
 * @param  self		éè¦æ§å¶å¯¹è±¡çæé
 * @return NULL 
 */
void kinematics_init(KinematicsObjectTypeDef* self);

/**
 * @brief éè¿å¨å­¦è§£ç®
 * 
 * @param  self		éè¦æ§å¶å¯¹è±¡çæé
 * @return  K_OK		æè§£
 * 			INVAILD	æ è§£
 */
uint8_t ikine(KinematicsObjectTypeDef* self);

/**
 * @brief æ­£è¿å¨å­¦è§£ç®
 * 
 * @param  knot0_theta	ä»ä¸è³ä¸ç¬¬1ä¸ªå³èè§åº¦
 * @param  knot1_theta	ä»ä¸è³ä¸ç¬¬2ä¸ªå³èè§åº¦
 * @param  knot2_theta	ä»ä¸è³ä¸ç¬¬3ä¸ªå³èè§åº¦
 * @param  knot3_theta	ä»ä¸è³ä¸ç¬¬4ä¸ªå³èè§åº¦
 * @return VectorObjectTypeDefç±»åç»æä½
 */
VectorObjectTypeDef fkine(float knot0_theta, float knot1_theta, float knot2_theta, float knot3_theta);

#endif
