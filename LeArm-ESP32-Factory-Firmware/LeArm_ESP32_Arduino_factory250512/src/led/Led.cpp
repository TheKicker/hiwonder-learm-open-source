#include "./../../Hiwonder.hpp"
#include <Arduino.h>

#define LED_TASK_PERIOD     ((float)30) /* LED状态刷新间隔(ms) */

#define BATTERY_TASK_PERIOD ((float)50) /* 电池电量检测间隔(ms) */

static void led_control_callback(Led_t* obj)
{
    /* 尝试从队列中取的新的控制数据， 如果成功取出则重置状态机重新开始一个控制循环 */
    if(obj->new_flag != 0) {
        obj->new_flag = 0;
        obj->stage = LED_STAGE_START_NEW_CYCLE;
    }
    /* 状态机处理 */
    switch(obj->stage) {
        case LED_STAGE_START_NEW_CYCLE: {
            if(obj->ticks_on > 0) {
                digitalWrite(obj->led_pin,LOW);
                if(obj->ticks_off > 0) { /* 熄灭时间不为 0 即为 闪烁否则为长亮 */
                    obj->ticks_count = 0;
                    obj->stage = LED_STAGE_WATTING_OFF; /* 等待 LED 灯亮起时间结束 */
                }else{
                  obj->stage = LED_STAGE_IDLE; /* 长亮， 转入空闲 */
                }
            } else { /* 只要亮起时间为 0 即为长灭 */
                digitalWrite(obj->led_pin,HIGH);
				        obj->stage = LED_STAGE_IDLE; /* 长灭， 转入空闲 */
            }
            break;
        }
        case LED_STAGE_WATTING_OFF: {
            obj->ticks_count += LED_TASK_PERIOD;
            if(obj->ticks_count >= obj->ticks_on) { /* LED 亮起时间结束 */
                digitalWrite(obj->led_pin,HIGH);
                obj->stage = LED_STAGE_WATTING_PERIOD_END;
            }
            break;
        }
        case LED_STAGE_WATTING_PERIOD_END: { /* 等待周期结束 */
            obj->ticks_count += LED_TASK_PERIOD;
            if(obj->ticks_count >= (obj->ticks_off + obj->ticks_on)) {
				        obj->ticks_count -= (obj->ticks_off + obj->ticks_on);
                if(obj->repeat == 1) { /* 剩余重复次数为1时就可以结束此次控制任务 */
                    digitalWrite(obj->led_pin,HIGH);
                    obj->stage = LED_STAGE_IDLE;  /* 重复次数用完， 转入空闲 */
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

