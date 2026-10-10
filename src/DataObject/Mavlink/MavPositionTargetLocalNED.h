/*
 * MavPositionTargetLocalNED.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavPositionTargetLocalNED__H_
#define OpenKAI_src__DataObject__Mavlink__MavPositionTargetLocalNED__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavPositionTargetLocalNED : public MavMsgBase
	{
	public:
		MavPositionTargetLocalNED();

		// for send message to IO
		const MAV_MSG_TSTAMP& add(mavlink_position_target_local_ned_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		const mavlink_position_target_local_ned_t &get() const { return m_msg; }

	protected:
		mavlink_position_target_local_ned_t m_msg{};
	};

}
#endif
