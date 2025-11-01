#ifndef __PC_BLE_CTL_HPP_
#define __PC_BLE_CTL_HPP_

#include "Robot_arm.hpp"

#define LEARM_VERSION 0x01

#define APP_PACKET_HEADER                  0x55  /* éä¿¡åè®®å¸§å¤´ */
#define APP_TX_DATA_LENGTH                   2  /* é¤å»å¸§å¤´çåéç«¯æ°æ®é¿åº¦ */
#define MAX_PACKET_LENGTH					64

typedef enum
{
	PACKET_HEADER_1 = 0,
	PACKET_HEADER_2,
	PACKET_DATA_LENGTH,
	PACKET_FUNCTION,
	PACKET_DATA
}PacketAnalysisStatus;

typedef enum
{
	APP_VERSION_QUERY = 1, //åºä»¶æ¥è¯¢
	APP_SERVO_OFFSET_READ, //èµæºåå·®è¯»å
	APP_MULT_SERVO_MOVE, //æ§å¶å/å¤ä¸ªèµæºèå®½
	APP_COORDINATE_SET, //åæ æ§å¶

	APP_ACTION_GROUP_RUN = 6, //å¨ä½ç»è¿è¡
	APP_FULL_ACTION_STOP, //åæ­¢æ­£å¨è¿è¡çå¨ä½ç»
	APP_FULL_ACTION_ERASE, //å°ä¸è½½å°æ§å¶æ¿çå¨ä½ç»æ¦é¤
	APP_CHASSIS_CONTROL,// åºçæ§å¶
	APP_SERVO_OFFSET_SET, //èµæºåå·®è®¾ç½®
	APP_SERVO_OFFSET_DOWNLOAD,//èµæºåå·®ä¸è½½
	APP_SERVOS_RESET, //å¤ä½ä½å§¿
	APP_SERVOS_READ, //è¯»åèµæºè§åº¦
	
	APP_ACTION_DOWNLOAD = 25, //å¨ä½ç»ä¸è½½
	APP_FUNC_NULL
}AppFunctionStatus;


#pragma pack(1)
typedef struct
{
	uint8_t action_frame_sum;
	uint8_t action_frame_index;
	uint8_t action_group_index;
	uint16_t running_times;
	
	uint8_t packet_header[2];
	uint8_t data_len;
	uint8_t cmd;
	uint8_t buffer[MAX_PACKET_LENGTH - 4];
}PacketObjectTypeDef;
#pragma pack()

class PC_BLE_CTL
{
private:
	uint8_t set_id;
	uint8_t servos_count;
	uint16_t set_duty;
	uint16_t running_time;

	bool unpack_successful;
    PacketObjectTypeDef tmp_packet;
    PacketObjectTypeDef packet;
	PacketAnalysisStatus packet_status;
	AppFunctionStatus status;

	struct{
		float x;
		float y;
		float z;
		int8_t pitch;
	}pose;

	void unpack(void);
	void packet_transmit(uint8_t* data, uint8_t len);
	uint8_t transmit_data(const uint8_t* pdata, uint16_t size);


public:
	void init(int pc_ble_flag);
	void PC_BLE_Task(LeArm_t* robot,Led_t* led,Buzzer_t* buzzer);
};


#endif //__PC_BLE_CTL_HPP_
