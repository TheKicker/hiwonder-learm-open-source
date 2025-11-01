#include <Arduino.h>
#include "Hiwonder.hpp"

#define BUZZER_TASK_PERIOD  ((float)30) /* èé¸£å¨ç¶æå·æ°é´é(ms) */

static void buzzer_control_callback(Buzzer_t* obj)
{
  /* å°è¯ä»éåä¸­åçæ°çæ§å¶æ°æ®ï¼ å¦ææåååºåéç½®ç¶ææºéæ°å¼å§ä¸ä¸ªæ§å¶å¾ªç¯ */
    if(obj->new_flag != 0) {
        obj->new_flag = 0;
        obj->stage = BUZZER_STAGE_START_NEW_CYCLE;
    }
    /* ç¶ææºå¤ç */
    switch(obj->stage) {
        case BUZZER_STAGE_START_NEW_CYCLE: {
            if(obj->ticks_on > 0 && obj->freq > 0) {
                ledcWriteTone(obj->buzzer_channel , obj->freq); /* é¸£åèé¸£å¨ */
                if(obj->ticks_off > 0) {/* éé³æ¶é´ä¸ä¸º 0 å³ä¸º ååå å¦åå°±æ¯é¿é¸£ */
                    obj->ticks_count = 0;
                    obj->stage = BUZZER_STAGE_WATTING_OFF; /* ç­å°é¸£åæ¶é´ç»æ */
                }else{
					obj->stage = BUZZER_STAGE_IDLE; /* é¿é¸£ï¼è½¬å¥ç©ºé² */
				}
            } else { /* åªè¦é¸£åæ¶é´ä¸º 0 å³ä¸ºéé³ */
                ledcWriteTone(obj->buzzer_channel , 0);
				obj->stage = BUZZER_STAGE_IDLE;  /* é¿éé³ï¼è½¬å¥ç©ºé² */
            }
            break;
        }
        case BUZZER_STAGE_WATTING_OFF: {
            obj->ticks_count += BUZZER_TASK_PERIOD;
            if(obj->ticks_count >= obj->ticks_on) { /* é¸£åæ¶é´ç»æ */
                ledcWriteTone(obj->buzzer_channel , 0);
                obj->stage = BUZZER_STAGE_WATTING_PERIOD_END;
            }
            break;
        }
        case BUZZER_STAGE_WATTING_PERIOD_END: { /* ç­å¾å¨æç»æ */
            obj->ticks_count += BUZZER_TASK_PERIOD;
            if(obj->ticks_count >= (obj->ticks_off + obj->ticks_on)) {
                obj->ticks_count -= (obj->ticks_off + obj->ticks_on);
                if(obj->repeat == 1) { /* å©ä½éå¤æ¬¡æ°ä¸º1æ¶å°±å¯ä»¥ç»ææ­¤æ¬¡æ§å¶ä»»å¡ */
                    ledcWriteTone(obj->buzzer_channel , 0);
                    obj->stage = BUZZER_STAGE_IDLE;
                } else {
                    ledcWriteTone(obj->buzzer_channel , obj->freq);
                    obj->repeat = obj->repeat == 0 ? 0 : obj->repeat - 1;
                    obj->stage = BUZZER_STAGE_WATTING_OFF;
                }
            }
            break;
        }
        case BUZZER_STAGE_IDLE: {
            break;
        }
        default:
            break;
    }
}


void Buzzer_t::init(uint8_t pin , uint8_t channel , uint16_t frequency)
{
    buzzer_pin = pin;
    stage = BUZZER_STAGE_IDLE;
    ticks_count = 0;
    freq = frequency;
    buzzer_channel = channel;
    ledcSetup(buzzer_channel,frequency,12);
    ledcAttachPin(buzzer_pin, buzzer_channel);
    ledcWrite(buzzer_channel, 2048);
    ledcWriteTone(buzzer_channel , 0);
    timer_buzzer.attach((BUZZER_TASK_PERIOD/1000), buzzer_control_callback , this);
}

void Buzzer_t::on_off(uint8_t state)
{
  if(state != 0)
  {
    blink(1500 , 200 , 400 , 0);
  }else{
    blink(1500 , 0 , 400 , 0);
  }
}

void Buzzer_t::blink(uint16_t frequency , uint16_t on_time , uint16_t off_time , uint16_t count)
{
    new_flag = 1;
    freq = frequency;
    ticks_on = on_time;
    ticks_off = off_time;
    repeat = count;
}
