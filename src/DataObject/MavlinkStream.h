/*
 * MavlinkStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__MavlinkStream__H_
#define OpenKAI_src__DataStream__MavlinkStream__H_

#include "DataObjBase.h"

namespace kai
{
	class MavMsgBase;

	struct MAVLINK_MSG
	{
		MavMsgBase* m_pMavMsg = nullptr;
		uint64_t m_tStamp = 0;

		void clear(void)
		{
			m_pMavMsg = nullptr;
			m_tStamp = 0;
		}

		void set(MavMsgBase& mavMsg, uint64_t tStamp = 0)
		{
			m_pMavMsg = &mavMsg;
			updateTstamp(tStamp);
		}

		MavMsgBase* get(void)
		{
			return m_pMavMsg;
		}

		void updateTstamp(uint64_t tStamp = 0)
		{
			if (tStamp == 0)
				m_tStamp = getTns();
			else
				m_tStamp = tStamp;
		}

		uint64_t getTstamp(void)
		{
			return m_tStamp;
		}
	};

	class MavlinkStream : public DataObjBase
	{
	public:
		MavlinkStream();
		virtual ~MavlinkStream();
		bool loadConfig(void);
		bool saveConfig(bool bExport = false);

		void console(void *pConsole) override;

		bool clear(size_t nMsg = 0);
		void addMsg(MavMsgBase& msg, uint64_t tStamp = 0);
		void getMsgs(vector<MAVLINK_MSG>& vMsg, uint64_t tStampFrom = 0);

	protected:
		vector<MAVLINK_MSG> m_vMsg;
		int m_nMsg = 0;		// number of msg in vector to be reserved
		int m_iMset = 0;	// index of msg ring buf written
		std::shared_mutex m_sMutex;
	};

}
#endif
