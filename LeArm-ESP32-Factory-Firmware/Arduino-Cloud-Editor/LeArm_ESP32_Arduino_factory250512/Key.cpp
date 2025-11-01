#include "Hiwonder.hpp"
#include <Arduino.h>
#include "Config.h"

#define BUTTON_TASK_PERIOD  ((float)30) /* æ¿è½½æé®æ«æé´é(ms) */

static void button_scan(Button_t* obj, uint8_t id)
{
    obj->bt_run[id-1].ticks_count += BUTTON_TASK_PERIOD;

    uint32_t pin = obj->read(id);
    if(pin != obj->bt_run[id-1].last_pin_raw)  { /* ååè¿ç»­çä¸¤æ¬¡IOç¶æä¸åè®¤ä¸ºæé®ç¶æè¿ä¸ç¨³å®ï¼ä¿å­æ°çIOç¶æç¶åè¿å */
        obj->bt_run[id-1].last_pin_raw = pin;
        return;
    }

	  /* æé®ç¶ææ²¡ææ¹å, å³ç¶ææºç¶æä¸ä¼åçè½¬ç§», ç´æ¥è¿å */
    if(obj->bt_run[id-1].last_pin_filtered == obj->bt_run[id-1].last_pin_raw && obj->bt_run[id-1].stage == BUTTON_STAGE_NORMAL && obj->bt_run[id-1].combin_counter == 0) { 
        return;
    }

    obj->bt_run[id-1].last_pin_filtered = obj->bt_run[id-1].last_pin_raw; /* ä¿å­æ°çæé®ç¶æ */
    switch(obj->bt_run[id-1].stage) {
        case BUTTON_STAGE_NORMAL: {
            if(obj->bt_run[id-1].last_pin_filtered) {
                obj->event_callback(id, BUTTON_EVENT_PRESSED); /* è§¦åæé®æä¸äºä»¶ */
                if(obj->bt_run[id-1].ticks_count < obj->bt_run[id-1].combin_th && obj->bt_run[id-1].combin_counter > 0) { /* åªæå¨è¿å»è®¡æ°ä¸ä¸ºé¶æ¶è¿å»æèµ·ä½ç¨ */
                    obj->bt_run[id-1].combin_counter += 1;
                    if(obj->bt_run[id-1].combin_counter == 2) {  /* åå»åè° */
                        obj->event_callback(id, BUTTON_EVENT_DOUBLE_CLICK);
                    }
                    if(obj->bt_run[id-1].combin_counter == 3) {  /* ä¸è¿å»åè° */
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
                if(obj->bt_run[id-1].ticks_count > obj->bt_run[id-1].lp_th) { /* è¶è¿é¿æè§¦åæ¶é´ */
                    obj->event_callback(id, BUTTON_EVENT_LONGPRESS); /* è§¦åé¿æäºä»¶ */
                    obj->bt_run[id-1].ticks_count = 0;
                    obj->bt_run[id-1].stage = BUTTON_STAGE_LONGPRESS; /* ç¶æè½¬ä¸ºé¿æ */
                }
            } else { /* æé®æ¾å¼ */
                obj->event_callback(id, BUTTON_EVENT_RELEASE_FROM_SP); /* è§¦åç­ææ¾å¼äºä»¶ */
                obj->event_callback(id, BUTTON_EVENT_CLICK);  /* è§¦åç¹å»æ¾å¼äºä»¶ */
                obj->bt_run[id-1].combin_counter = obj->bt_run[id-1].combin_counter == 0 ? 1 : obj->bt_run[id-1].combin_counter; /* åªæå¨è¿å»è®¡æ°ä¸ä¸ºé¶æ¶è¿å»æèµ·ä½ç¨ */
                obj->bt_run[id-1].stage = BUTTON_STAGE_NORMAL;
            }
            break;
		    }
        case BUTTON_STAGE_LONGPRESS: {
            if(obj->bt_run[id-1].last_pin_filtered) {
                if(obj->bt_run[id-1].ticks_count > obj->bt_run[id-1].repeat_th)  {
                    obj->event_callback(id, BUTTON_EVENT_LONGPRESS_REPEAT); /* è§¦åé¿æéå¤éå¤äºä»¶ */
                    obj->bt_run[id-1].ticks_count = 0; /* éæ°è®¡æ¶ä¸ä¸æ¬¡éå¤è§¦å */
                }
            } else { /* æé®æ¾å¼ */
                obj->event_callback(id, BUTTON_EVENT_RELEASE_FROM_LP);  /* è§¦åé¿ææ¾å¼äºä»¶ */
                obj->bt_run[id-1].combin_counter = 0;                /* é¿æä¸å¯è¿å», è¿å»è®¡æ°ä¸º0æ¶è¿å»è®¡æ°ä¸çæ */
                obj->bt_run[id-1].ticks_count = obj->bt_run[id-1].combin_th + 1; /* é¿æä¸å¯è¿å», è®©è¿å»è®¡æ¶ç´æ¥è¶æ¶ */
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

