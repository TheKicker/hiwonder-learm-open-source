#include "Hiwonder.hpp"
#include <Arduino.h>

#define LED_TASK_PERIOD     ((float)30) /* LEDç¶æå·æ°é´é(ms) */

#define BATTERY_TASK_PERIOD ((float)50) /* çµæ± çµéæ£æµé´é(ms) */

static void led_control_callback(Led_t* obj)
{
    /* å°è¯ä»éåä¸­åçæ°çæ§å¶æ°æ®ï¼ å¦ææåååºåéç½®ç¶ææºéæ°å¼å§ä¸ä¸ªæ§å¶å¾ªç¯ */
    if(obj->new_flag != 0) {
        obj->new_flag = 0;
        obj->stage = LED_STAGE_START_NEW_CYCLE;
    }
    /* ç¶ææºå¤ç */
    switch(obj->stage) {
        case LED_STAGE_START_NEW_CYCLE: {
            if(obj->ticks_on > 0) {
                digitalWrite(obj->led_pin,LOW);
                if(obj->ticks_off > 0) { /* çç­æ¶é´ä¸ä¸º 0 å³ä¸º éªçå¦åä¸ºé¿äº® */
                    obj->ticks_count = 0;
                    obj->stage = LED_STAGE_WATTING_OFF; /* ç­å¾ LED ç¯äº®èµ·æ¶é´ç»æ */
                }else{
                  obj->stage = LED_STAGE_IDLE; /* é¿äº®ï¼ è½¬å¥ç©ºé² */
                }
            } else { /* åªè¦äº®èµ·æ¶é´ä¸º 0 å³ä¸ºé¿ç­ */
                digitalWrite(obj->led_pin,HIGH);
				        obj->stage = LED_STAGE_IDLE; /* é¿ç­ï¼ è½¬å¥ç©ºé² */
            }
            break;
        }
        case LED_STAGE_WATTING_OFF: {
            obj->ticks_count += LED_TASK_PERIOD;
            if(obj->ticks_count >= obj->ticks_on) { /* LED äº®èµ·æ¶é´ç»æ */
                digitalWrite(obj->led_pin,HIGH);
                obj->stage = LED_STAGE_WATTING_PERIOD_END;
            }
            break;
        }
        case LED_STAGE_WATTING_PERIOD_END: { /* ç­å¾å¨æç»æ */
            obj->ticks_count += LED_TASK_PERIOD;
            if(obj->ticks_count >= (obj->ticks_off + obj->ticks_on)) {
				        obj->ticks_count -= (obj->ticks_off + obj->ticks_on);
                if(obj->repeat == 1) { /* å©ä½éå¤æ¬¡æ°ä¸º1æ¶å°±å¯ä»¥ç»ææ­¤æ¬¡æ§å¶ä»»å¡ */
                    digitalWrite(obj->led_pin,HIGH);
                    obj->stage = LED_STAGE_IDLE;  /* éå¤æ¬¡æ°ç¨å®ï¼ è½¬å¥ç©ºé² */
                } else {
                    digitalWrite(obj->led_pin,LOW);
                    obj->repeat = obj->repeat == 0 ? 0 : obj->repeat - 1;
                    obj->stage = LED_STAGE_WATTING_OFF;
                }
            }
            break;
        }
        case LED_STAGE_IDLE: {
            break;
        }
        default:
            break;
    }
}

void Led_t::init(uint8_t pin)
{
    led_pin = pin;
    stage = LED_STAGE_IDLE;
    ticks_count = 0;
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin,HIGH);
    timer_led.attach((LED_TASK_PERIOD/1000), led_control_callback , this);
}

void Led_t::on_off(uint8_t state)
{
    if(state)
    {
        blink(100 , 0 , 0);
    }else{
        blink(0 , 100 , 0);
    }
}

void Led_t::blink(uint32_t on_time , uint32_t off_time , uint32_t count)
{
    new_flag = 1;
    ticks_on = on_time;
    ticks_off = off_time;
    repeat = count;
}

