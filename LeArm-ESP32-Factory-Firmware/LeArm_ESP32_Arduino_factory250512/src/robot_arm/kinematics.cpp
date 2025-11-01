#include "kinematics.hpp"
#include "math.h"
#include "string.h"

float theta2rad(float theta)
{
	return theta * PI / 180.0f;
}

float rad2theta(float rad)
{
	return rad * 180.0f / PI;
}

void kinematics_init(KinematicsObjectTypeDef* self)
{
	memset(self, 0, sizeof(KinematicsObjectTypeDef));
}

VectorObjectTypeDef fkine(float knot1_theta, float knot2_theta, float knot3_theta, float knot4_theta)
{
	float length, height;
	VectorObjectTypeDef vector;
	
	length = LINKAGE_2 * cos(theta2rad(knot2_theta)) + \
					 LINKAGE_3 * cos(theta2rad(knot2_theta) + theta2rad(knot3_theta)) + \
					 LINKAGE_4 * cos(theta2rad(knot3_theta) + theta2rad(knot3_theta) + theta2rad(knot4_theta));
	
	height = LINKAGE_2 * sin(theta2rad(knot2_theta)) + \
					 LINKAGE_3 * sin(theta2rad(knot2_theta) + theta2rad(knot3_theta)) + \
					 LINKAGE_4 * sin(theta2rad(knot3_theta) + theta2rad(knot3_theta) + theta2rad(knot4_theta)) + LINKAGE_1;

	vector.x = length * sin(theta2rad(knot1_theta));
	vector.y = length * cos(theta2rad(knot1_theta));
	vector.z = height;
	
	return vector;
}

/*
 * -90 <= knot[0].theta <= 90
 * 0 <= knot[1].theta <= 180
 * -90 <= knot[2].theta <= 90
 * -90 <= knot[3].theta <= 90 
 *
 */

uint8_t ikine(KinematicsObjectTypeDef* self)
{
	float a, b, c, d, length, cos_knot2, sin_knot2;
	KnotObjectTypeDef knots[4];
	
	length = sqrt(self->vector.x * self->vector.x + self->vector.y * self->vector.y);
	a = length - (LINKAGE_4 * cos(theta2rad(self->alpha)));
	b = self->vector.z - LINKAGE_1 - (LINKAGE_4 * sin(theta2rad(self->alpha)));
	
	knots[0].rad = atan2(self->vector.y, self->vector.x);
	knots[0].theta = rad2theta(knots[0].rad);
	
	cos_knot2 = ((a * a) + (b * b) - (LINKAGE_2 * LINKAGE_2) - (LINKAGE_3 * LINKAGE_3)) / (2 * LINKAGE_2 * LINKAGE_3);
	if(cos_knot2 > 1.0f)
	{
		return INVAILD;
	}
	sin_knot2 = -sqrt(1 - (cos_knot2 * cos_knot2));
	knots[2].rad = atan2(sin_knot2, cos_knot2);
	knots[2].theta = rad2theta(knots[2].rad);

	c = LINKAGE_2 + LINKAGE_3 * cos(knots[2].rad);
	d = LINKAGE_3 * sin(knots[2].rad);
	knots[1].rad = atan2(c, d) - atan2(a, b);
	knots[1].theta = rad2theta(knots[1].rad);
	
	knots[3].theta = self->alpha - knots[1].theta - knots[2].theta;
	knots[3].rad = theta2rad(knots[3].theta);
	
	if ((knots[0].theta > MIN_KNOT6_ANGLE) && (knots[0].theta < MAX_KNOT6_ANGLE) &&	\
		(knots[1].theta > MIN_KNOT5_ANGLE) && (knots[1].theta < MAX_KNOT5_ANGLE) &&	\
		(knots[2].theta > MIN_KNOT4_ANGLE) && (knots[2].theta < MAX_KNOT4_ANGLE) &&	\
		(knots[3].theta > MIN_KNOT3_ANGLE) && (knots[3].theta < MAX_KNOT3_ANGLE))
	{
		for(uint8_t i = 0; i < 4; i++)
		{
			self->knot[i].rad = knots[i].rad;
			self->knot[i].theta = knots[i].theta;
		}
		return K_OK;
	}
	else
	{
		return INVAILD;
	}
}
