#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_RTCM_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_RTCM_H_

#include "../../Protocol/_Mavlink.h"
#include "../../Navigation/_RTCMcast.h"

#define GPS_DATA_FRAG_N 180

namespace kai
{

	class _APmavlink_RTCM : public _RTCMcast
	{
	public:
		_APmavlink_RTCM();
		~_APmavlink_RTCM();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		virtual bool writeMavlink(RTCM_MSG* pM);
		virtual void writeMsg(void);
		virtual void handleMsg(const RTCM_MSG &msg);

	protected:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_APmavlink_RTCM *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_APmavlink_RTCM *)This)->updateR();
			return NULL;
		}

	protected:
		_Mavlink* m_pMav;

		uint8_t m_iSeq;
	};

}
#endif
