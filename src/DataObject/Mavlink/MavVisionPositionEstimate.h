/*
 * MavVisionPositionEstimate.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavVisionPositionEstimate__H_
#define OpenKAI_src__DataObject__Mavlink__MavVisionPositionEstimate__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavVisionPositionEstimate : public MavMsgBase
	{
	public:
		MavVisionPositionEstimate();

		// for send message to IO
		const MAV_MSG_TSTAMP& add(mavlink_vision_position_estimate_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		const mavlink_vision_position_estimate_t &get() const { return m_msg; }

	protected:
		mavlink_vision_position_estimate_t m_msg{};
	};

}
#endif
