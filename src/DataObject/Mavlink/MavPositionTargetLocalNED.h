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
		void add(mavlink_position_target_local_ned_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

	protected:
		mavlink_position_target_local_ned_t m_msg{};
	};

}
#endif
