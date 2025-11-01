#include "./../../Hiwonder.hpp"
#include <Arduino.h>
#include "./../../Config.h"

#define BUTTON_TASK_PERIOD  ((float)30) /* 板载按键扫描间隔(ms) */

static void button_scan(Button_t* obj, uint8_t id)
{
    obj->bt_run[id-1].ticks_count += BUTTON_TASK_PERIOD;

    uint32_t pin = obj->read(id);
    if(pin != obj->bt_run[id-1].last_pin_raw)  { /* 前后连续的两次IO状态不同认为按钮状态还不稳定，保存新的IO状态然后返回 */
        obj->bt_run[id-1].last_pin_raw = pin;
        return;
    }

	  /* 按钮状态没有改变, 即状态机状态不会发生转移, 直接返回 */
    if(obj->bt_run[id-1].last_pin_filtered == obj->bt_run[id-1].last_pin_raw && obj->bt_run[id-1].stage == BUTTON_STAGE_NORMAL && obj->bt_run[id-1].combin_counter == 0) { 
        return;
    }

    obj->bt_run[id-1].last_pin_filtered = obj->bt_run[id-1].last_pin_raw; /* 保存新的按钮状态 */
    switch(obj->bt_run[id-1].stage) {
        case BUTTON_STAGE_NORMAL: {
            if(obj->bt_run[id-1].last_pin_filtered) {
                obj->event_callback(id, BUTTON_EVENT_PRESSED); /* 触发按键按下事件 */
                if(obj->bt_run[id-1].ticks_count < obj->bt_run[id-1].combin_th && obj->bt_run[id-1].combin_counter > 0) { /* 只有在连击计数不为零时连击才起作用 */
                    obj->bt_run[id-1].combin_counter += 1;
                    if(obj->bt_run[id-1].combin_counter == 2) {  /* 双击回调 */
                        obj->event_callback(id, BUTTON_EVENT_DOUBLE_CLICK);
                    }
                    if(obj->bt_run[id-1].combin_counter == 3) {  /* 三连击回调 */
                        obj->event_callback(id, BUTTON_EVENT_TRIPLE_CLICK);
                    }
                }
                obj->bt_run[id-1].ticks_count = 0;
                obj->bt_run[id-1].stage = BUTTON_STAGE_PRESS;
            } else {
                if(obj->bt_run[id-1].ticks_count > obj->bt_run[id-1].combin_th && obj->bt_run[id-1].combin_counter != 0) {
                    obj->bt_run[id-1].combin_counter = 0;
                    obj->bt_run[id-1].ticks_count = 0;
                }
            }
            break;
		    }
        case BUTTON_STAGE_PRESS: {
            if(obj->bt_run[id-1].last_pin_filtered) {
                if(obj->bt_run[id-1].ticks_count > obj->bt_run[id-1].lp_th) { /* 超过长按触发时间 */
                    obj->event_callback(id, BUTTON_EVENT_LONGPRESS); /* 触发长按事件 */
                    obj->bt_run[id-1].ticks_count = 0;
                    obj->bt_run[id-1].stage = BUTTON_STAGE_LONGPRESS; /* 状态转为长按 */
                }
            } else { /* 按钮松开 */
                obj->event_callback(id, BUTTON_EVENT_RELEASE_FROM_SP); /* 触发短按松开事件 */
                obj->event_callback(id, BUTTON_EVENT_CLICK);  /* 触发点击松开事件 */
                obj->bt_run[id-1].combin_counter = obj->bt_run[id-1].combin_counter == 0 ? 1 : obj->bt_run[id-1].combin_counter; /* 只有在连击计数不为零时连击才起作用 */
                obj->bt_run[id-1].stage = BUTTON_STAGE_NORMAL;
            }
            break;
		    }
        case BUTTON_STAGE_LONGPRESS: {
            if(obj->bt_run[id-1].last_pin_filtered) {
                if(obj->bt_run[id-1].ticks_count > obj->bt_run[id-1].repeat_th)  {
                    obj->event_callback(id, BUTTON_EVENT_LONGPRESS_REPEAT); /* 触发长按重复重复事件 */
                    obj->bt_run[id-1].ticks_count = 0; /* 重新计时下一次重复触发 */
                }
            } else { /* 按钮松开 */
                obj->event_callback(id, BUTTON_EVENT_RELEASE_FROM_LP);  /* 触发长按松开事件 */
                obj->bt_run[id-1].combin_counter = 0;                /* 长按不可连击, 连击计数为0时连击计数不生效 */
                obj->bt_run[id-1].ticks_count = obj->bt_run[id-1].combin_th + 1; /* 长按不可连击, 让连击计时直接超时 */
                obj->bt_run[id-1].stage = BUTTON_STAGE_NORMAL;
            }
            break;
		    }
    }
}

static void button_control_callback(Button_t* obj)
{
  button_scan(obj , 1);
  button_scan(obj , 2);
}

void button_defalut_event_callback(uint8_t id,  ButtonEventIDEnum event)
{
}

void Button_t::init(void)
{
    for(int i = 0 ; i < 2 ; i++){
      bt_run[i].stage = BUTTON_STAGE_NORMAL;
      bt_run[i].last_pin_raw = 0;
      bt_run[i].last_pin_filtered = 0;
      bt_run[i].combin_counter = 0;
      bt_run[i].ticks_count = 0;

      /* config */
      bt_run[i].lp_th = 2000;  
      bt_run[i].repeat_th = 500;
      bt_run[i].combin_th = 300;
      bt_run[i].lp_th = 1500;  
		  bt_run[i].repeat_th = 400;
    }
    event_callback = button_defalut_event_callback; 
    timer_button.attach((BUTTON_TASK_PERIOD/1000), button_control_callback , this);
}

uint8_t Button_t::read(uint8_t id)
{
  int adcValue = analogRead(IO_BUTTON);
  uint8_t button_id = 0;
  if(700 < adcValue && adcValue < 900)
  {
    button_id = 1;
  }else if(1600 < adcValue && adcValue < 2000)
  {
    button_id = 2;
  }else if(500 < adcValue && adcValue < 700)
  {
    button_id = 3;
  }
  if(id == 1 && (button_id == 1 || button_id == 3))
  {
    return 1;
  }else if(id == 2 && (button_id == 2 || button_id == 3))
  {
    return 1;
  }else{
    return 0;
  }
}

void Button_t::register_callback(void (*function)(uint8_t , ButtonEventIDEnum))
{
  event_callback = function;
}

