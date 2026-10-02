/*
 * MavServoOutputRaw.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavServoOutputRaw.h"

namespace kai
{

	MavServoOutputRaw::MavServoOutputRaw()
	{
		m_id = MAVLINK_MSG_ID_SERVO_OUTPUT_RAW;

		clearMsgQueue();
	}

	void MavServoOutputRaw::add(mavlink_servo_output_raw_t &msg, uint8_t mySysID, uint8_t myComID)
	{
		mavlink_message_t msgT;
		mavlink_msg_servo_output_raw_encode(mySysID, myComID, &msgT, &msg);

		addMsgQueue(msgT);
	}

	void MavServoOutputRaw::decode(const mavlink_message_t &msg)
	{
		mavlink_msg_servo_output_raw_decode(&msg, &m_msg);
		updateTstamp();
		callbackAll();
	}

	uint16_t MavServoOutputRaw::getServo(int iServo)
	{
		switch (iServo)
		{
		case 1:
			return m_msg.servo1_raw;
		case 2:
			return m_msg.servo2_raw;
		case 3:
			return m_msg.servo3_raw;
		case 4:
			return m_msg.servo4_raw;
		case 5:
			return m_msg.servo5_raw;
		case 6:
			return m_msg.servo6_raw;
		case 7:
			return m_msg.servo7_raw;
		case 8:
			return m_msg.servo8_raw;
		case 9:
			return m_msg.servo9_raw;
		case 10:
			return m_msg.servo10_raw;
		case 11:
			return m_msg.servo11_raw;
		case 12:
			return m_msg.servo12_raw;
		case 13:
			return m_msg.servo13_raw;
		case 14:
			return m_msg.servo14_raw;
		case 15:
			return m_msg.servo15_raw;
		case 16:
			return m_msg.servo16_raw;
		default:
			return 0;
		}
	}

}
