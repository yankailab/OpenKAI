/*
 * MavButtonChange.h
 *
 *  Created on: Oct 27, 2026
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavButtonChange__H_
#define OpenKAI_src__DataObject__Mavlink__MavButtonChange__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavButtonChange : public MavMsgBase
	{
	public:
		MavButtonChange();

		// for send message to IO
		const MAV_MSG_TSTAMP& add(mavlink_button_change_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg) override;

		const mavlink_button_change_t &get() const { return m_msg; }

	protected:
		mavlink_button_change_t m_msg{};
	};

}
#endif
