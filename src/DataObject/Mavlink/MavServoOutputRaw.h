/*
 * MavServoOutputRaw.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavServoOutputRaw__H_
#define OpenKAI_src__DataObject__Mavlink__MavServoOutputRaw__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavServoOutputRaw : public MavMsgBase
	{
	public:
		MavServoOutputRaw();

		// for send message to IO
		const MAV_MSG_TSTAMP& add(mavlink_servo_output_raw_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		uint16_t getServo(int iServo);

		const mavlink_servo_output_raw_t &get() const { return m_msg; }

	protected:
		mavlink_servo_output_raw_t m_msg{};
	};

}
#endif
