#include "Hiwonder.hpp"
#include <Arduino.h>
#include "Config.h"

#include "Servo.h"
#include <esp_timer.h>

typedef struct {
  uint8_t pin_id;          /**< ESP32 GPIO pin ID for the servo. ->èµæºå¼è*/
  int current_pulsewidth;  /**< Current pulse width of the servo. ->å½åèå®½*/
  int actual_pulsewidth;   /**< Actual pulse width of the servo. --å®éè¿è¡èå®½ï¼å·²å offsetï¼*/
  bool pulsewidth_changed; /**< Flag indicating whether the pulse width has changed. ->èå®½ååæ å¿ä½*/
  bool is_running;         /**< Flag indicating whether the servo is running. ->èµæºæ¯å¦å¤å¨è¿è¡ä¸­çæ å¿ä½*/
  int target_pulsewidth;   /**< Target pulse width of the servo. ->ç®æ èå®½*/
  int offset;              /**< Offset value for the servo. ->èµæºåå·®*/
  float pulsewidth_inc;    /**< Increment of pulse width per cycle (20ms). ->æ¯å¨æèå®½çå¢é*/
  uint32_t duration;       /**< Duration of the servo movement. Unit: ms ->èµæºè¿è¡æ¶é´*/
  int inc_num;             /**< Number of increments of pulse width. ->èå®½å¢éæ°*/

} pwm_servo_obj_t;

static void pwm_servo_pulsewidth_update(pwm_servo_obj_t *self, int8_t index);
static void timer_update_callback(void *argv);

DRAM_ATTR pwm_servo_obj_t pwm_servos[6] = {
    {.pin_id = SERVO_6, .current_pulsewidth = SERVO6_RESET_DUTY, .actual_pulsewidth = SERVO6_RESET_DUTY, .pulsewidth_changed = false, .is_running = false, .target_pulsewidth = SERVO6_RESET_DUTY},
    {.pin_id = SERVO_5, .current_pulsewidth = SERVO5_RESET_DUTY, .actual_pulsewidth = SERVO5_RESET_DUTY, .pulsewidth_changed = false, .is_running = false, .target_pulsewidth = SERVO5_RESET_DUTY},
    {.pin_id = SERVO_4, .current_pulsewidth = SERVO4_RESET_DUTY, .actual_pulsewidth = SERVO4_RESET_DUTY, .pulsewidth_changed = false, .is_running = false, .target_pulsewidth = SERVO4_RESET_DUTY},
    {.pin_id = SERVO_3, .current_pulsewidth = SERVO3_RESET_DUTY, .actual_pulsewidth = SERVO3_RESET_DUTY, .pulsewidth_changed = false, .is_running = false, .target_pulsewidth = SERVO3_RESET_DUTY},
    {.pin_id = SERVO_2, .current_pulsewidth = SERVO2_RESET_DUTY, .actual_pulsewidth = SERVO2_RESET_DUTY, .pulsewidth_changed = false, .is_running = false, .target_pulsewidth = SERVO2_RESET_DUTY},
    {.pin_id = SERVO_1, .current_pulsewidth = SERVO1_RESET_DUTY, .actual_pulsewidth = SERVO1_RESET_DUTY, .pulsewidth_changed = false, .is_running = false, .target_pulsewidth = SERVO1_RESET_DUTY},
};

Servo servos[6];
esp_timer_handle_t timer_handle;
static uint8_t servo_deinit_flag = 0;

/**
  * @brief Update the pulsewidth of the PWM servo.
  *
  * This function is used to update the pulsewidth of the PWM servo.
  * The pulsewidth is updated gradually until it reaches the target pulsewidth.
  *
  * @param self The PWM servo object.
  *
  * @return None.
*/
static void IRAM_ATTR pwm_servo_pulsewidth_update(pwm_servo_obj_t *self, int8_t index) {
  // Check if pulsewidth has changed
  if (self->pulsewidth_changed) {
    self->pulsewidth_changed = false;

    // Calculate pulsewidth increment based on target and current pulsewidth
    if (self->current_pulsewidth == 0) {  // If current pulsewidth is 0, set increment to target pulsewidth
      self->pulsewidth_inc = (float)self->target_pulsewidth;
      self->inc_num = 1;
    } else {
      self->inc_num = self->duration / 10;  // 10ms per tick
      if (self->target_pulsewidth > self->current_pulsewidth) {
        self->pulsewidth_inc = (float)(-(self->target_pulsewidth - self->current_pulsewidth)) / (float)self->inc_num;
      } else {
        self->pulsewidth_inc = (float)(self->current_pulsewidth - self->target_pulsewidth) / (float)self->inc_num;
      }
    }
    self->is_running = true;
  }

  // Update pulsewidth gradually until it reaches the target pulsewidth
  if (self->is_running) {
    --self->inc_num;
    if (self->inc_num == 0) {
      self->current_pulsewidth = self->target_pulsewidth;  // Update current pulsewidth to target pulsewidth
      self->is_running = false;
    } else {
      self->current_pulsewidth = self->target_pulsewidth + (int)(self->pulsewidth_inc * self->inc_num);
    }
    // Calculate actual pulsewidth based on current pulsewidth and offset
    self->actual_pulsewidth = self->current_pulsewidth + self->offset;
    // Set the compare value of the hardware comparator
    servos[index].writeMicroseconds(self->actual_pulsewidth);
  }
}

static void IRAM_ATTR timer_update_callback(void *argv) {
  if (servo_deinit_flag == 0) {
    for (int i = 0; i < SERVO_NUM; ++i) {
      pwm_servo_pulsewidth_update(&pwm_servos[i], i);
    }
  }
}

/* PWMèµæºæ»æ§å¶å¨ */
void PwmServo_t::init(void)
{
  /*è¯»åPWMèµæºåå·®ï¼æªå®æï¼*/

  for (int i = 0; i < SERVO_NUM; ++i) {
    servos[i].attach(pwm_servos[i].pin_id);
    servos[i].writeMicroseconds(pwm_servos[i].actual_pulsewidth);
  }
  // Set up timer to update pulsewidth
  const esp_timer_create_args_t timer_args = {
    .callback = &timer_update_callback,  // timer callback function
    .name = "pwm_servo_update_timer",
  };

  esp_timer_create(&timer_args, &timer_handle);
  esp_timer_start_periodic(timer_handle, 10000);
}

int  PwmServo_t::set_duty(uint16_t servo_index, uint32_t pulsewidth, uint32_t duration)
{
  // Check if servo ID is valid
  if (servo_index >= SERVO_NUM) {
    return -1;
  }
  duration = duration < 20 ? 20 : (duration > 30000 ? 30000 : duration);  // Limit duration to 20ms to 30s
  pulsewidth = pulsewidth > 2500 ? 2500 : (pulsewidth < 500 ? 500 : pulsewidth); // Limit pulsewidth to 500us to 2500us
  pwm_servos[servo_index].target_pulsewidth = pulsewidth;  // Set target pulsewidth
  pwm_servos[servo_index].duration = duration;             // Set duration
  pwm_servos[servo_index].pulsewidth_changed = true;
  return 0;
}

int  PwmServo_t::set_angle(uint16_t servo_index, uint32_t angle, uint32_t duration)
{
  uint32_t pulsewidth = 1500;
  // Check if servo ID is valid
  if (servo_index >= SERVO_NUM) {
    return -1;
  }
  angle = angle > 180 ? 180 : angle;
  pulsewidth = angle * 2000 / 180 + 500;
  pulsewidth = pulsewidth > 2500 ? 2500 : (pulsewidth < 500 ? 500 : pulsewidth); // Limit pulsewidth to 500us to 2500us
  duration = duration < 20 ? 20 : (duration > 30000 ? 30000 : duration);  // Limit duration to 20ms to 30s
  pwm_servos[servo_index].target_pulsewidth = pulsewidth;  // Set target pulsewidth
  pwm_servos[servo_index].duration = duration;             // Set duration
  pwm_servos[servo_index].pulsewidth_changed = true;
  return 0;
}

int  PwmServo_t::read_duty(uint16_t servo_index)
{
  // Check if servo ID is valid
  if (servo_index >= SERVO_NUM) {
    return -1;
  }
  return pwm_servos[servo_index].current_pulsewidth;
}

int  PwmServo_t::read_angle(uint16_t servo_index)
{
  // Check if servo ID is valid
  if (servo_index >= SERVO_NUM) {
    return -1;
  }
  return ((pwm_servos[servo_index].duration-500)*180/2000);
}

int  PwmServo_t::set_offset(uint16_t servo_index, int offset)
{
  // Check if servo ID is valid
  if (servo_index >= SERVO_NUM) {
    return -1;
  }
  // Check if offset is within the valid range of -125 to 125
  if (offset < -125 || offset > 125) {
    return -2;
  }
  pwm_servo_obj_t *self = &pwm_servos[servo_index];
  self->offset = offset;
  self->actual_pulsewidth = self->current_pulsewidth + self->offset;
  servos[servo_index].writeMicroseconds(self->actual_pulsewidth);
  return 0;
}

int  PwmServo_t::read_offset(uint16_t servo_index)
{
  if (servo_index >= SERVO_NUM) {
    return 0;
  }
  return pwm_servos[servo_index].offset;
}

int  PwmServo_t::stop(uint16_t servo_index)
{
  set_duty(servo_index ,pwm_servos[servo_index].current_pulsewidth, 0);
  return 0;
}

bool  PwmServo_t::is_ready(uint16_t servo_index)
{
    // Check if servo ID is valid
    if (servo_index >= SERVO_NUM) {
      return false;
    }
    if(pwm_servos[servo_index].actual_pulsewidth != (pwm_servos[servo_index].target_pulsewidth+pwm_servos[servo_index].offset))
    {
      return false;
    }else{
      return true;
    }
}

void PwmServo_t::deinit(void)
{
  esp_timer_stop(timer_handle);
  servo_deinit_flag = 1;
  for (int i = 0; i < SERVO_NUM; ++i) {
    servos[i].detach();
  }
}
