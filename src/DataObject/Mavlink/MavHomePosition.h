/*
 * MavHomePosition.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavHomePosition__H_
#define OpenKAI_src__DataObject__Mavlink__MavHomePosition__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavHomePosition : public MavMsgBase
	{
	public:
		MavHomePosition();

		// for send message to IO
		void add(mavlink_home_position_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		const mavlink_home_position_t &get() const { return m_msg; }

	protected:
		mavlink_home_position_t m_msg{};
	};

}
#endif
