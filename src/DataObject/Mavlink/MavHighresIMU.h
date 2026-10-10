/*
 * MavHighresIMU.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavHighresIMU__H_
#define OpenKAI_src__DataObject__Mavlink__MavHighresIMU__H_

#include "MavMsgBase.h"

namespace kai
{
	class MavHighresIMU : public MavMsgBase
	{
	public:
		MavHighresIMU();

		// for send message to IO
		const MAV_MSG_TSTAMP& add(mavlink_highres_imu_t &msg, uint8_t mySysID, uint8_t myComID);

		// for decode message received from IO
		void decode(const mavlink_message_t &msg);

		const mavlink_highres_imu_t &get() const { return m_msg; }

	protected:
		mavlink_highres_imu_t m_msg{};
	};

}
#endif
