#include "PS2_CTL.hpp"
#include "Robot_arm.hpp"
#include "Hiwonder.hpp"
#include "Config.h"

#define Serial_PS2 Serial2
TaskHandle_t PS2TaskHandle;

float map_x, map_y, map_z;

static uint16_t MAX_DUTY = 2500, MIN_DUTY = 500;

static float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return out_min + (x - in_min) * ((out_max - out_min) / (in_max - in_min));
}

void PS2_CTL::init(void)
{
  Serial_PS2.begin(9600 ,SERIAL_8N1 , USB_TX , -1);
  action_running_time = 1000;
}

void PS2_CTL::PS2_Task(LeArm_t* robot,Led_t* led,Buzzer_t* buzzer)
{
  if(robot->get_servo_type() == 1){
    MAX_DUTY = 875;
    MIN_DUTY = 125;
  }
  static uint8_t first_flag = 1;
  if(first_flag!=0)
  {
    first_flag = 0;
    // if(mode_count == PS2_SINGLE_SERVO_MODE)
    // {
    //   led->blink(250,250,0);
    // }else{
    //   led->blink(250,0,0);
    // }
  }
  receive_msg();
  get_result(robot,led,buzzer);
}

void PS2_CTL::clear_rec(void)
{
    while (Serial_PS2.available()>0)
    {
        Serial_PS2.read();
    }
}

void PS2_CTL::receive_msg(void)
{
  static uint8_t step = 0;
  static uint8_t index = 0;
  while (Serial_PS2.available()>0) {
    switch(step){
      case 0:{
        if(Serial_PS2.read() == FRAME_HEADER)
        {
          step++;
        }
      }break;
      case 1:{
        if(Serial_PS2.read() == FRAME_HEADER)
        {
          index = 0;
          step++;
        }else{
          step = 0;
        }
      }break;
      case 2:{
        recbuff[index++] = Serial_PS2.read();
        if(index > 9)
        {
          rec_flag = true;
          step = 0;
        }
      }break;
      default:{
        step = 0;
      }break;
    }

    if(rec_flag)
    {
      Serial.print("rec_flag:");
      for(int i = 0 ; i < 10 ; i++)
      {
        Serial.print(recbuff[i],HEX);
        Serial.print(" ");
      }
      Serial.println();
      
      if(recbuff[9] == 0x80 && recbuff[8] == 0x80)
      {
        keyvalue.mode = PS2_SINGLE_SERVO_MODE;
      }else{
        keyvalue.mode = PS2_COORDINATE_MODE;
      }

      keyvalue.bit_triangle = 	recbuff[3] & 0xFF;
      keyvalue.bit_circle = 	(recbuff[3] & 0xFF) >> 1;
      keyvalue.bit_cross = 		(recbuff[3] & 0xFF) >> 2;
      keyvalue.bit_square = 	(recbuff[3] & 0xFF) >> 3;
      keyvalue.bit_l1 =  		(recbuff[3] & 0xFF) >> 4;
      keyvalue.bit_r1 =  		(recbuff[3] & 0xFF) >> 5;
      keyvalue.bit_l2 =  		(recbuff[3] & 0xFF) >> 6;
      keyvalue.bit_r2 =  		(recbuff[3] & 0xFF) >> 7;
      keyvalue.bit_select = 	recbuff[4] & 0xFF;
      keyvalue.bit_start = 		(recbuff[4] & 0xFF) >> 1;
      keyvalue.bit_leftjoystick_press =  (recbuff[4] & 0xFF) >> 2;
      keyvalue.bit_rightjoystick_press = (recbuff[4] & 0xFF) >> 3;

      switch(keyvalue.mode){
        case PS2_SINGLE_SERVO_MODE:
          keyvalue.bit_left = recbuff[6] == 0x00 ? 1 : 0;
          keyvalue.bit_right = recbuff[6] == 0xFF ? 1 : 0;
          keyvalue.bit_up = recbuff[7] == 0x00 ? 1 : 0;
          keyvalue.bit_down = recbuff[7] == 0xFF ? 1 : 0;
          break;
        
        case PS2_COORDINATE_MODE:
          keyvalue.left_joystick_x = recbuff[6];
          keyvalue.left_joystick_y = recbuff[7];
          keyvalue.right_joystick_x = recbuff[8];
          keyvalue.right_joystick_y = recbuff[9];
          switch(recbuff[5])
          {
            case 0x00:
              keyvalue.bit_up = 1;
              break;
            
            case 0x01:
              keyvalue.bit_up = 1;
              keyvalue.bit_right = 1;
              break;
            
            case 0x02:
              keyvalue.bit_right = 1;
              break;
            
            case 0x03:
              keyvalue.bit_down = 1;
              keyvalue.bit_right = 1;
              break;
            
            case 0x04:
              keyvalue.bit_down = 1;
              break;
            
            case 0x05:
              keyvalue.bit_down = 1;
              keyvalue.bit_left = 1;
              break;
            
            case 0x06:
              keyvalue.bit_left = 1;
              break;
            
            case 0x07:
              keyvalue.bit_up = 1;
              keyvalue.bit_left = 1;
              break;
            
            case 0x0F:
              keyvalue.bit_up = 0;
              keyvalue.bit_down = 0;
              keyvalue.bit_left = 0;
              keyvalue.bit_right = 0;
              break;
          }
          break;
      }
      break;
    }
  }
}


int PS2_CTL::get_result(LeArm_t* robot,Led_t* led,Buzzer_t* buzzer)
{
  if(rec_flag){
    rec_flag = false;
    if(keyvalue.mode == PS2_SINGLE_SERVO_MODE)
    {
      Serial.println("single");
      if(keyvalue.bit_select != 1)
      {
        if(keyvalue.bit_triangle == 1)
        {
// #if (SERVO_TYPE == TYPE_PWM_SERVO)
if(robot->get_servo_type() == 0){
          robot->knot_run(1, 1500, action_running_time);
// #else
}else{
          robot->knot_run(1, 700, action_running_time);
}
// #endif
        }
        else if(keyvalue.bit_cross == 1)
        {
          robot->knot_run(1, MIN_DUTY, action_running_time);
        }
        else if(keyvalue.bit_triangle == 0 || keyvalue.bit_cross == 0)
        {
          if(last_keyvalue.bit_triangle != 0 || last_keyvalue.bit_cross != 0)
          {
            robot->knot_stop(1);
          }
        }

        if(keyvalue.bit_square == 1)
        {
          robot->knot_run(2, MIN_DUTY, action_running_time);
        }
        else if(keyvalue.bit_circle == 1)
        {
          robot->knot_run(2, MAX_DUTY, action_running_time);
        }
        else if(keyvalue.bit_square == 0 || keyvalue.bit_circle == 0)
        {
          if(last_keyvalue.bit_square != 0 || last_keyvalue.bit_circle != 0)
          {
            robot->knot_stop(2);
          }
        }

        if(keyvalue.bit_r1 == 1)
        {
          robot->knot_run(3, MAX_DUTY, action_running_time);
        }
        else if(keyvalue.bit_r2 == 1)
        {
          robot->knot_run(3, MIN_DUTY, action_running_time);
        }
        else if(keyvalue.bit_r1 == 0 || keyvalue.bit_r2 == 0)
        {
          if(last_keyvalue.bit_r1 != 0 || last_keyvalue.bit_r2 != 0)
          {
            robot->knot_stop(3);
          }
        }

        if(keyvalue.bit_l1 == 1)
        {
          robot->knot_run(4, MAX_DUTY, action_running_time);
        }
        else if(keyvalue.bit_l2 == 1)
        {
          robot->knot_run(4, MIN_DUTY, action_running_time);
        }
        else if(keyvalue.bit_l1 == 0 || keyvalue.bit_l2 == 0)
        {
          if(last_keyvalue.bit_l1 != 0 || last_keyvalue.bit_l2 != 0)
			    {
            robot->knot_stop(4);
          }
        }
        
        if(keyvalue.bit_up == 1)
        {
          robot->knot_run(5, MAX_DUTY, action_running_time);
        }
        else if(keyvalue.bit_down == 1)
        {
          robot->knot_run(5, MIN_DUTY, action_running_time);
        }
        else if(keyvalue.bit_up == 0 || keyvalue.bit_down == 0)
        {
          if(last_keyvalue.bit_up != 0 || last_keyvalue.bit_down != 0)
			    {
            robot->knot_stop(5);
          }
        }

        if(keyvalue.bit_left == 1)
        {
          robot->knot_run(6, MAX_DUTY, action_running_time);
        }
        else if(keyvalue.bit_right == 1)
        {
          robot->knot_run(6, MIN_DUTY, action_running_time);
        }
        else if(keyvalue.bit_left == 0 || keyvalue.bit_right == 0)
        {
          if(last_keyvalue.bit_left != 0 || last_keyvalue.bit_right != 0)
          {
            robot->knot_stop(6);
          }
        }
      }

      if(keyvalue.bit_start == 1)
      {
        robot->reset();
        delay(800);
      }

      // å¨ä½ç»æ§å¶
      if(keyvalue.bit_select == 1)
      {
        buzzer->blink(1500 , 100, 100, 1);
        if(keyvalue.bit_up == 1)
        {
          robot->action_run(0, 1);
        }else if(keyvalue.bit_down == 1)
        {
          robot->action_run(1, 1);
        }else if(keyvalue.bit_left == 1)
        {
          robot->action_run(2, 1);
        }else if(keyvalue.bit_right == 1)
        {
          robot->action_run(3, 1);
        }else if(keyvalue.bit_l1 == 1)
        {
          robot->action_run(4, 1);
        }else if(keyvalue.bit_l2 == 1)
        {
          robot->action_run(5, 1);
        }else if(keyvalue.bit_triangle == 1)
        {
          robot->action_run(6, 1);
        }else if(keyvalue.bit_cross == 1)
        {
          robot->action_run(7, 1);
        }else if(keyvalue.bit_square == 1)
        {
          robot->action_run(8, 1);
        }else if(keyvalue.bit_circle == 1)
        {
          robot->action_run(9, 1);
        }else if(keyvalue.bit_r1 == 1)
        {
          robot->action_run(10, 1);
        }else if(keyvalue.bit_r2 == 1)
        {
          robot->action_run(11, 1);
        }
      }

      if(keyvalue.bit_leftjoystick_press)
      {
        buzzer->blink(1500 , 100, 100, 1);
        led->blink(100, 100, 1);
        if(action_running_time > 400)
        {
          action_running_time  -= 200;
        }
      }
      
      if(keyvalue.bit_rightjoystick_press)
      {
        buzzer->blink(1500 , 100, 100, 1);
        led->blink(100, 100, 1);
        if(action_running_time < 10000)
        {
          action_running_time += 200;
        }
      }
    }
    else if(keyvalue.mode == PS2_COORDINATE_MODE)
    {
      Serial.println("coord");
      map_x = map((float)((uint32_t)keyvalue.left_joystick_y), 0.0f, 255.0f, -10.0f, 10.0f);
      map_y = map((float)((uint32_t)keyvalue.left_joystick_x), 0.0f, 255.0f, -10.0f, 10.0f);
      map_z = map((float)((uint32_t)keyvalue.right_joystick_y), 0.0f, 255.0f, -10.0f, 10.0f);
      robot->coordinate_set(15 - map_x, -map_y, 15 - map_z, 0 , -90, 90, 500);
    }
  }
  last_keyvalue = keyvalue;
  return 0;
}

