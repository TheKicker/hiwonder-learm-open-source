#include "Robot_arm.hpp"

/* ç¨äºä¿å­å·²æçå¨ä½ç»ä¸­åèªçæ»å¨ä½å¸§æ°é */
uint8_t af_sum[ACTION_GROUP_MAX_NUM];	

static float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return out_min + (x - in_min) * ((out_max - out_min) / (in_max - in_min));
}

static bool set_pitch_range(KinematicsObjectTypeDef* self, VectorObjectTypeDef* vector, float alpha1, float alpha2)
{
	float inc;
	self->vector.x = vector->x;
	self->vector.y = vector->y;
	self->vector.z = vector->z;
	inc = (alpha1 > alpha2) ? -1.0f : 1.0f;
	
	for (self->alpha = alpha1;
		 (inc > 0) ? (self->alpha < alpha2) : (self->alpha >= alpha2);
		 self->alpha += inc)
	{
		if (ikine(self) == K_OK)
		{
			return true;
		}
	}
	return false;
}

void LeArm_t::theta2servo(KinematicsObjectTypeDef* self, float time)
{
	float target_angle[4] = {0};
	target_angle[0] = 90.0f + self->knot[0].theta;
	target_angle[1] = 180.0f - self->knot[1].theta;
	target_angle[2] = 90.0f + self->knot[2].theta;
	target_angle[3] = 90.0f + self->knot[3].theta;
	for (uint8_t i = 0; i < 4; i++)
	{
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
    	pwmservo_obj.set_angle(i, target_angle[i], time);
}
// #else
else{
	target_angle[i] = map(target_angle[i], 0, 180, 125, 875);
	busservo_obj.set_position(6-i, target_angle[i], time);
}
// #endif
	}
}

/* 
 * 6å·èµæº å·¦+ å³-
 * 5å·èµæº å+ å-
 * 4å·èµæº å- å+
 * 3å·èµæº å- å+
 */
uint8_t LeArm_t::coordinate_set(float target_x,float target_y,float target_z,float pitch,float min_pitch,float max_pitch,uint32_t time)
{
	bool result1_state, result2_state;
	
	KinematicsObjectTypeDef kinematics_result1;
	KinematicsObjectTypeDef kinematics_result2;	
	VectorObjectTypeDef vector;

	vector.x = target_x;
	vector.y = target_y;
	vector.z = target_z;
	
	result1_state = set_pitch_range(&kinematics_result1, &vector, pitch, min_pitch);
	result2_state = set_pitch_range(&kinematics_result2, &vector, pitch, max_pitch);
	
	if (result1_state)
	{
		kinematics.alpha = kinematics_result1.alpha;
		kinematics.vector.x = kinematics_result1.vector.x;
		kinematics.vector.y = kinematics_result1.vector.y;
		kinematics.vector.z = kinematics_result1.vector.z;
		for (uint8_t i = 0; i< 4; i++)
		{
			kinematics.knot[i].theta = kinematics_result1.knot[i].theta;
		}
		
		if (result2_state)
		{
			if (fabs(kinematics_result2.alpha - pitch) < fabs(kinematics_result1.alpha - pitch))
			{
				kinematics.alpha = kinematics_result2.alpha;
				kinematics.vector.x = kinematics_result2.vector.x;
				kinematics.vector.y = kinematics_result2.vector.y;
				kinematics.vector.z = kinematics_result2.vector.z;
				for (uint8_t i = 0; i< 4; i++)
				{
					kinematics.knot[i].theta = kinematics_result2.knot[i].theta;
				}			
			}
		}
	}
	else
	{
		if (result2_state)
		{
			kinematics.alpha = kinematics_result2.alpha;
			kinematics.vector.x = kinematics_result2.vector.x;
			kinematics.vector.y = kinematics_result2.vector.y;
			kinematics.vector.z = kinematics_result2.vector.z;
			for (uint8_t i = 0; i< 4; i++)
			{
				kinematics.knot[i].theta = kinematics_result2.knot[i].theta;
			}
		}
		else
		{
			return false;
		}
	}
	result1_state = 0;
	result2_state = 0;
	theta2servo(&kinematics, time);
	return true;
}

/*
 * 2å·èµæº å³è½¬- å·¦è½¬+
 */
void LeArm_t::roll_set(float rotation_angle, uint32_t rotation_angle_time)
{
	float target_rotation_angle;
	rotation_angle = rotation_angle > MAX_ROTATION_ANGLE ? MAX_ROTATION_ANGLE : \
					(rotation_angle < MIN_ROTATION_ANGLE ? MIN_ROTATION_ANGLE : rotation_angle);
	
	target_rotation_angle = 90.0f - rotation_angle;
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
  pwmservo_obj.set_angle(4, target_rotation_angle, rotation_angle_time);
}
// #else
else{
	target_rotation_angle = map(target_rotation_angle, 0, 180, 30, 210);
	busservo_obj.set_angle(2, target_rotation_angle, rotation_angle_time);
}
// #endif
}

/*
 * 1å·èµæº å¼ å¼- é­å+
 */
void LeArm_t::claw_set(float open_angle, uint32_t open_angle_time)
{
	float target_open_angle;
	
	open_angle = open_angle > MAX_OPEN_ANGLE ? MAX_OPEN_ANGLE : \
				(open_angle < MIN_OPEN_ANGLE ? MIN_OPEN_ANGLE : open_angle);
	target_open_angle = 90.0f - open_angle;
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
  pwmservo_obj.set_angle(5, target_open_angle, open_angle_time);
}
// #else
else{
	target_open_angle = map(target_open_angle, 0, 90, 200, 700);
	busservo_obj.set_position(1, target_open_angle, open_angle_time);
}
// #endif
}

void LeArm_t::knot_run(uint8_t id, int target_duty, uint32_t time)
{
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
  if(id == 1 && target_duty > 1500)
  {
    target_duty = 1500;
  }
  pwmservo_obj.set_duty(6-id, target_duty, time);
}
// #else
else{
  if(id == 1)
  {
    target_duty = target_duty > 700 ? 700 : (target_duty < 200? 200 : target_duty);
  }else{
	  target_duty = target_duty > 875? 875 : (target_duty < 125? 125 : target_duty);
  }	busservo_obj.set_position(id, target_duty, time);
}
// #endif
}

void LeArm_t::knot_stop(uint8_t id)
{
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
    pwmservo_obj.stop(6-id);
}
// #else
else{
    busservo_obj.stop(id);
}
// #endif
}

uint16_t LeArm_t::knot_read(uint8_t id)
{
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
	uint16_t duty = pwmservo_obj.read_duty(6-id);
	return duty;
}
// #else
else{
	return busservo_obj.read_position(id);
}
// #endif
}

// #if (SERVO_TYPE == TYPE_PWM_SERVO)
bool LeArm_t::knot_finish(uint8_t id)
{
if(servo_type == 0){
    return pwmservo_obj.is_ready(6-id);
}
// #else
else{
	return true;
}
// #endif
}

void LeArm_t::action_group_erase()
{
	/* å°ææå¨ä½ç»çå¨ä½å¸§æ°éè®¾ç½®ä¸º0ï¼å³ä»£è¡¨å°ææå¨ä½ç»æ¦é¤ */
    memset(af_sum, 0, sizeof(af_sum));
    flash_obj.erase_sector(ACTION_FRAME_SUM_BASE_ADDRESS);
    flash_obj.write(ACTION_FRAME_SUM_BASE_ADDRESS,(uint8_t*)af_sum,sizeof(af_sum));
    delay(5);
}

void LeArm_t::action_group_init()
{
	uint8_t read_logo[9] = {0};
	const uint8_t logo[] = "Hiwonder";
	uint8_t offset_val[6] = {0,0,0,0,0,0};

    flash_obj.read(LOGO_BASE_ADDRESS, read_logo, sizeof(read_logo));
	flash_obj.read(ACTION_FRAME_SUM_BASE_ADDRESS, af_sum, sizeof(af_sum));
	for (uint8_t i = 0; i < sizeof(read_logo); i++)
	{
		if (read_logo[i] != logo[i])
		{
			flash_obj.write(ACTION_FRAME_SUM_BASE_ADDRESS,(uint8_t*)af_sum,sizeof(af_sum));
            flash_obj.erase_sector(LOGO_BASE_ADDRESS);
            flash_obj.write(LOGO_BASE_ADDRESS, (uint8_t*)logo, sizeof(logo));
			flash_obj.erase_sector(SERVOS_OFFSET_BASE_ADDRESS);	
			flash_obj.write(SERVOS_OFFSET_BASE_ADDRESS, offset_val, sizeof(offset_val));
			action_group_erase();
            return;
		}
	}
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
	flash_obj.read(SERVOS_OFFSET_BASE_ADDRESS, offset_val, sizeof(offset_val));
	for(uint8_t i = 0; i < 6; i++)
	{
		offset_set(i+1, (int8_t)offset_val[i]);
	}
}
// #endif
}

/* ä¸ä¸ªçå¨ä½å¸§çæ°ç»åå®¹
 * æ§å¶çèµæºæ°éï¼frame[0]
 * è¿è¡æ¶é´ï¼frame[1] + frame[2] << 8
 * èµæºidï¼frame[3 + i * 3]:
 * èµæºèå®½ï¼frame[4 + i * 3] + frame[5 + i * 3] << 8
 * 0-å¨ä½å¸§è¿è¡å¤±è´¥ 1-å¨ä½å¸§è¿è¡å®æ     0100 1101 0001
 */
uint8_t frame[ACTION_FRAME_SIZE];
uint8_t LeArm_t::action_frame_run(uint8_t action_group_index, uint8_t frame_index)
{
	uint8_t set_id;
	uint8_t control_servos_sum;
	
	uint16_t set_duty_num;
	uint32_t ag_addr_offset;
	uint32_t af_addr_offset;

	switch(robot_arm.action_group.frame.status)
	{
		case ACTION_FRAME_START:
			ag_addr_offset = action_group_index * ACTION_GROUP_SIZE;
			af_addr_offset = frame_index * ACTION_FRAME_SIZE;
			flash_obj.read(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset,
						frame, sizeof(frame));
			control_servos_sum = frame[0];

			if (control_servos_sum > SERVO_NUM)
			{
				return ACTION_FRAME_START;
			}
			robot_arm.action_group.frame.time = MERGE_HL(frame[2], frame[1]);

			for (uint8_t i = 0; i < control_servos_sum; i++)
			{
				set_id = frame[3 + i * 3];
				set_duty_num = (uint16_t)MERGE_HL(frame[5 + i * 3], frame[4 + i * 3]);
				knot_run(set_id, (int)set_duty_num, robot_arm.action_group.frame.time);
			}
			delay(10);

			robot_arm.action_group.frame.status = ACTION_FRAME_RUNNING;
			break;
		
		case ACTION_FRAME_RUNNING:
			delay(100);
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
      /* åä¸ä¸ªèµæºæ¥å¤æ­æ¯å¦æ¥å°è¾¾æå®ä½ç½® */
			if (pwmservo_obj.is_ready(0) && pwmservo_obj.is_ready(1) && pwmservo_obj.is_ready(2)
          && pwmservo_obj.is_ready(3) && pwmservo_obj.is_ready(4) && pwmservo_obj.is_ready(5))
      	{
			robot_arm.action_group.frame.index++;
			robot_arm.action_group.frame.status = ACTION_FRAME_IDLE;
		}
}
// #else
else{
      static uint32_t tick_time = 0;
      tick_time += 100;
      if (tick_time >= robot_arm.action_group.frame.time)
      {
        tick_time = 0;
// #endif
				robot_arm.action_group.frame.index++;
				robot_arm.action_group.frame.status = ACTION_FRAME_IDLE;
			}
}
			break;
		
		case ACTION_FRAME_IDLE:
			break;
		
		default:
			break;
	}

	return robot_arm.action_group.frame.status;
}

bool LeArm_t::action_group_run(uint8_t action_group_index, uint8_t running_times)
{
	// while(robot_arm.action_group.status != ACTION_GROUP_IDLE)
	// {
	static bool state = true;
		switch (robot_arm.action_group.status)
		{
			case ACTION_GROUP_START:
				state = false;
				robot_arm.action_group.running_times = running_times;
				robot_arm.action_group.index = action_group_index;
				robot_arm.action_group.frame.index = 0;
				flash_obj.read(ACTION_FRAME_SUM_BASE_ADDRESS + action_group_index,
							&robot_arm.action_group.sum, 1);
				/* å¦æè¯¥å¨ä½ç»çå¨ä½å¸§æ°éå¤§äº0ï¼åè¯´æå·²ç»ä¸è½½è¿å¨ä½ */
				robot_arm.action_group.status = robot_arm.action_group.sum > 0 ? \
												ACTION_GROUP_RUNNING : ACTION_GROUP_IDLE;
				break;

			case ACTION_GROUP_RUNNING:
				if(action_frame_run(robot_arm.action_group.index, robot_arm.action_group.frame.index) == ACTION_FRAME_IDLE)
				{
					if (robot_arm.action_group.frame.index == robot_arm.action_group.sum)
					{
						robot_arm.action_group.status = ACTION_GROUP_END_PERIOD;
						robot_arm.action_group.frame.index = 0;
					}
					else
					{
						robot_arm.action_group.frame.status = ACTION_FRAME_START;
					}
				}
				/* code */
				break;

			case ACTION_GROUP_END_PERIOD:
				if(robot_arm.action_group.running_times <= 1)
				{
					robot_arm.action_group.status = ACTION_GROUP_IDLE;
				}
				else
				{
					--robot_arm.action_group.running_times;
					robot_arm.action_group.status = ACTION_GROUP_RUNNING;
				}
				break;

			case ACTION_GROUP_IDLE:
				state = true;
				break;
			
			default:
				break;
		}
		return state;
	// }
}

void LeArm_t::action_run(uint8_t action_group_index, uint8_t repeat_times)
{
	while(!action_group_run(action_group_index , repeat_times));
	action_group_reset();
}

void LeArm_t::action_group_reset()
{
	robot_arm.action_group.status = ACTION_GROUP_START;
}

void LeArm_t::action_group_stop()
{
	for (uint8_t i = 0; i < SERVO_NUM; i++)
	{
		knot_stop(i);
	}
	robot_arm.action_group.status = ACTION_GROUP_IDLE;
	robot_arm.action_group.frame.status = ACTION_FRAME_START;
}

// uint8_t write_frame[8][ACTION_FRAME_SIZE] = {0};
// uint8_t read_frame[8][ACTION_FRAME_SIZE] = {0};
int LeArm_t::action_group_save(uint8_t action_group_index, uint8_t frame_num,uint8_t frame_index,uint8_t* pdata,uint16_t size)
{
	uint32_t ag_addr_offset;
	uint32_t af_addr_offset;
	uint32_t page_offset;
	uint32_t write_addr;
	uint16_t remaining_space;
	
	robot_arm.action_group.index = action_group_index;
	robot_arm.action_group.frame.index = frame_index;
	ag_addr_offset = action_group_index * ACTION_GROUP_SIZE;
	af_addr_offset = frame_index * ACTION_FRAME_SIZE;
	page_offset = af_addr_offset % 256; // æ­£ç¡®è®¡ç®é¡µåç§»
	write_addr = ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset;
	remaining_space = 256 - page_offset;
	
	/* å¦æåå¥å¯¹åºå¨ä½ç»çç¬¬ä¸å¸§ï¼åéè¦åæ¦é¤è¯¥å¨ä½ç»çææåå®¹ */
	if (frame_index == 0)
	{
		/* ä¸ä¸ªå¨ä½ç»å 8KBï¼æ¦é¤ä¸ä¸ªæåºæ¯4KBï¼åéè¦æ¦é¤2æ¬¡ */
		for (uint8_t i = 0; i < 2; i++)
		{
			flash_obj.erase_sector(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + (i * 4096));
		}
	}
	/* è·¨é¡µåå¥å¤çï¼é¿åé æåå¥å¤±è´¥ è¥è¦å¨é¡µå°¾ä¿è¯1å¸§æ°æ®å¨é¨åå¥æåï¼é£ä¹é¡µåç§»çå°åå¿é¡»ä¿è¯è¦ä¸å¤§äº234(255 - ä¸å¸§çå­èæ°21)*/
	// if(page_offset > 234)
	// {
	// 	flash_obj.write(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset, (uint8_t*)pdata, 256 - page_offset);
	// 	flash_obj.write(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset + (256 - page_offset), (uint8_t*)(pdata + (256 - page_offset)), size - (256 - page_offset));
	// }
	// else
	// {
	// 	flash_obj.write(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset, (uint8_t*)pdata, size);
	// }

	/* è·¨é¡µåå¥å¤ç */
    if (remaining_space < ACTION_FRAME_SIZE) {
        flash_obj.write(write_addr, pdata, remaining_space);
        flash_obj.write(write_addr + remaining_space, pdata + remaining_space, ACTION_FRAME_SIZE - remaining_space);
    }else{
        flash_obj.write(write_addr, pdata, ACTION_FRAME_SIZE);
    }

	// delay(5);
	// memcpy(write_frame[frame_index], pdata, size);
	if ((robot_arm.action_group.frame.index + 1) == frame_num)
	{
		/* å¦æåå¥çæ¯æåä¸å¸§ï¼æ­¤æ¶éè¦æ´æ°ä¸ä¸flashä¸­å¯¹åºå¨ä½ç»çå¨ä½å¸§æ»æ° */
		flash_obj.read(ACTION_FRAME_SUM_BASE_ADDRESS,af_sum, sizeof(af_sum));
		af_sum[robot_arm.action_group.index] = frame_num;
		flash_obj.erase_sector(ACTION_FRAME_SUM_BASE_ADDRESS);
		flash_obj.write(ACTION_FRAME_SUM_BASE_ADDRESS,(uint8_t*)af_sum,sizeof(af_sum));
		delay(5);
		return 1;
	}
	return 0;
}

// 0ï¼PWMèµæº ï¼ 1ï¼æ»çº¿èµæº
uint8_t LeArm_t::read_servo_type(void)
{
	for(int i = 0 ; i < 6 ; i++){
		if(busservo_obj.read_angle(1) != -2048){
			Serial.println("serial servo");
			return 1;
		}
		delay(50);
	}
	Serial.println("pwm servo");
	return 0;
}

void LeArm_t::init(void)
{
    flash_obj.init();

	Serial1.begin(115200 ,SERIAL_8N1 , BUS_RX , BUS_TX);
    busservo_obj.init(&Serial1);
	servo_type = read_servo_type();

// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
    pwmservo_obj.init();
// #else
}else{
	serial_servo_offset_init();
	reset(1000);
}
// #endif
	kinematics_init(&kinematics);
	memset(&robot_arm, 0, sizeof(RobotArmHandleTypeDef));
	action_group_init();
}

void LeArm_t::reset(uint32_t time)
{
if(servo_type == 0){
	knot_run(6, SERVO6_RESET_DUTY, time);
	knot_run(5, SERVO5_RESET_DUTY, time);
	knot_run(4, SERVO4_RESET_DUTY, time);
	knot_run(3, SERVO3_RESET_DUTY, time);
	knot_run(2, SERVO2_RESET_DUTY, time);
	knot_run(1, SERVO1_RESET_DUTY, time);
}else{
	knot_run(6, 500, time);
	knot_run(5, 408, time);
	knot_run(4, 129, time);
	knot_run(3, 177, time);
	knot_run(2, 500, time);
	knot_run(1, 226, time);
}
}

int8_t LeArm_t::offset_read(uint8_t id)
{
	if(id > 6 || id < 1)
	{
		return 0;
	}
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
	return (int8_t)pwmservo_obj.read_offset(6 - id);
}
// #else
else{
	return (int8_t)bus_servo_offset[id-1];
}
// #endif
}

void LeArm_t::offset_set(uint8_t id, int8_t value)
{
	if(id > 6 || id < 1)
	{
		return;
	}
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
	pwmservo_obj.set_offset(6-id, (int)value);
}
// #else
else{
	busservo_obj.SetDev(id, (int)value);
	bus_servo_offset[id-1] = (int8_t)value;
}
// #endif
}

void LeArm_t::offset_save(void)
{
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(servo_type == 0){
	uint8_t data[6];
	for(uint8_t i = 0; i < 6; i++)
	{
		data[i] = (uint8_t)offset_read(i+1);
	}
	flash_obj.erase_sector(SERVOS_OFFSET_BASE_ADDRESS);
	for(uint8_t i = 0; i < 6; i++)
	{
		flash_obj.write(SERVOS_OFFSET_BASE_ADDRESS, (uint8_t*)data, sizeof(data));
	}
// #else
}else{
	for(int i = 0 ; i < 6 ; i++){
		busservo_obj.SaveDev(i+1);
	}
}
// #endif
}

uint8_t LeArm_t::get_servo_type(void)
{
  return servo_type;
}

// #if (SERVO_TYPE == TYPE_SERIAL_SERVO)
void LeArm_t::serial_servo_offset_init(void)
{
if(servo_type == 1){
	for(int i = 0; i < 6; i++){
		bus_servo_offset[i] = busservo_obj.ReadDev(i+1);
	}
}
}
// #endif
