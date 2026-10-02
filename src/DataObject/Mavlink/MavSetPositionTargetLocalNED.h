/*
 * MavSetPositionTargetLocalNED.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavSetPositionTargetLocalNED__H_
#define OpenKAI_src__DataObject__Mavlink__MavSetPositionTargetLocalNED__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavSetPositionTargetLocalNED : public MavMsgBase
	{
	public:
		MavSetPositionTargetLocalNED();

		// for send message to IO
		void add(mavlink_set_position_target_local_ned_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

	protected:
		mavlink_set_position_target_local_ned_t m_msg{};
	};

}
#endif
