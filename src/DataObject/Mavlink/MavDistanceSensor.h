/*
 * MavDistanceSensor.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavDistanceSensor__H_
#define OpenKAI_src__DataObject__Mavlink__MavDistanceSensor__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavDistanceSensor : public MavMsgBase
	{
	public:
		MavDistanceSensor();

		// for send message to IO
		const MAV_MSG_TSTAMP& add(mavlink_distance_sensor_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		const mavlink_distance_sensor_t &get() const { return m_msg; }

	protected:
		mavlink_distance_sensor_t m_msg{};
	};

}
#endif
