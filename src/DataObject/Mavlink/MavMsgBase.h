/*
 * MavMsgBase.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataObject__Mavlink__MavMsgBase__H_
#define OpenKAI_src__DataObject__Mavlink__MavMsgBase__H_

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include "../../Dependencies/c_library_v2/ardupilotmega/mavlink.h"
#include "../../Dependencies/c_library_v2/mavlink_conversions.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "../DataObjBase.h"
#include <mutex>

namespace kai
{
	typedef void (*CbMavMsg)(void *pMsg, void *pInst);
	struct MavCallback
	{
		CbMavMsg m_pCbRecv = NULL;
		void *m_pCbInst = NULL;

		void callback(void *pMavMsg);
	};

	struct MAV_MSG_TSTAMP
	{
		mavlink_message_t m_msgT{};
		uint64_t m_tStamp = 0;
	};

	class MavMsgBase
	{
	public:
		MavMsgBase();

		virtual ~MavMsgBase(void);

		uint32_t getID(void);

		// common setter for decode message received from IO
		virtual void decode(const mavlink_message_t &msg);

		// validity and frequency
		bool bValid(void);
		void setDesiredInterval(int64_t tIntervalNsec);
		int64_t getDesiredInterval(void);
		bool bOnTime(void);

		// time stamp
		void updateTstamp(uint64_t tStamp = 0);
		uint64_t getTstamp(void);

		// callbacks
		void callbackAll(void);
		bool addCbRecv(CbMavMsg pCb, void *pInst);
		void clearCbRecv(CbMavMsg pCb, void *pInst);
		void clearAllCbRecv(void);

		// internal message queue
		void clearMsgQueue(size_t nM = 1);
		void addMsgQueue(mavlink_message_t& msgT);
		uint64_t getMsgQueue(vector<mavlink_message_t>& vMsg, uint64_t tStampFrom = 0);

	protected:
		// general
		uint32_t m_id = 0x7fffffff;
		uint64_t m_tStamp = 0;

		// frequency control
		int64_t m_tDesiredInterval = -1;
		int64_t m_tActualInterval = LONG_MAX;
		int64_t m_tIntervalDelayAllowed = NSEC_SEC / 10;

		// callback
		vector<MavCallback> m_vCbRecv;
		std::recursive_mutex m_cbMutex;

		// msg queue
		std::recursive_mutex m_sMutexMq;
		vector<MAV_MSG_TSTAMP> m_vMsgT{};
		size_t m_iMset = 0;
	};

}
#endif
