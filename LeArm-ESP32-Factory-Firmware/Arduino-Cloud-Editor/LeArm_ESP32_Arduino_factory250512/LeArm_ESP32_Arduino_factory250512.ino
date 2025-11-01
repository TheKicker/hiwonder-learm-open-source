#include "Config.h"
#include "Hiwonder.hpp"
#include "Robot_arm.hpp"
#include "PS2_CTL.hpp"
#include "PC_BLE_CTL.hpp"

Buzzer_t buzzer_obj;
Button_t key_obj;
Led_t led_obj;
LeArm_t arm;
PS2_CTL ps2;
PC_BLE_CTL pc_ble_obj;

uint8_t mode_flag = 0;
uint8_t button_2_flag = 0;

void button_change_mode(uint8_t id,  ButtonEventIDEnum event)
{
  if(event == BUTTON_EVENT_PRESSED)
  {
    if(id == 1)
    {
      if(mode_flag == 0)
      {
        // PCæ¨¡å¼
        mode_flag = 1;
        led_obj.blink(1000, 1000, 0);
        pc_ble_obj.init(0);
      }else if(mode_flag == 1){
        // PS2æææ¨¡å¼
        mode_flag = 2;
        led_obj.blink(200, 200, 0);
      }else if(mode_flag == 2){
        // è±æºæ¨¡å¼
        mode_flag = 3;
        led_obj.blink(50, 50, 0);
      }else{
        // APPæ¨¡å¼
        mode_flag = 0;
        led_obj.blink(2000, 2000, 0);
        pc_ble_obj.init(1);
      }
      buzzer_obj.blink(1500 , 100, 100, mode_flag+1);
    }
    if(id ==2)
    {
      if(mode_flag == 3){
        button_2_flag = 1;
        buzzer_obj.blink(1500 , 200, 50, 1);
      }
    }
  }
}

void setup() {
  delay(1000);
  pinMode(IO_BLE_CTL, OUTPUT);
  digitalWrite(IO_BLE_CTL, LOW);  // è®¾ç½®èçæ§å¶å¼èä¸ºä½çµå¹³æ¶ï¼æ­å¼èçæ¨¡åçµæº

  pinMode(PA4, OUTPUT);
  pinMode(PA5, OUTPUT);

  Serial.begin(9600);
  arm.init();
  led_obj.init(IO_LED);
  buzzer_obj.init(IO_BUZZER);
  key_obj.init();
  
  ps2.init();
  pc_ble_obj.init(1); // 0ï¼éæ©PCæ§å¶æ¨¡å¼

  ps2.init();
  delay(100);
  key_obj.register_callback(button_change_mode);
  led_obj.blink(2000, 2000, 0);
  if(arm.get_servo_type() == 0){
    buzzer_obj.blink(5000 , 50, 50, 1);
  }else{
    buzzer_obj.blink(5000 , 300, 50, 1);
  }
  Serial.println("begin");
  delay(1000);
}

void loop() {
  switch(mode_flag){
    case 0: // èçæ¨¡å¼
    case 1: // PCæ¨¡å¼
      pc_ble_obj.PC_BLE_Task(&arm , &led_obj , &buzzer_obj);
      break;
    case 2: // PS2æææ¨¡å¼
      ps2.PS2_Task(&arm, &led_obj, &buzzer_obj);
      break;
    case 3: // è±æºæ§å¶æ¨¡å¼
      if(button_2_flag != 0){
        arm.action_run(17,1);
        button_2_flag = 0;
      }
      break;
    default:
      break;
  }
  delay(10);
}
