/*
 * MavGpsRawINT.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavGpsRawINT__H_
#define OpenKAI_src__DataObject__Mavlink__MavGpsRawINT__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavGpsRawINT : public MavMsgBase
	{
	public:
		MavGpsRawINT();

		// for send message to IO
		void add(mavlink_gps_raw_int_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		const mavlink_gps_raw_int_t &get() const { return m_msg; }

	protected:
		mavlink_gps_raw_int_t m_msg{};
	};

}
#endif
