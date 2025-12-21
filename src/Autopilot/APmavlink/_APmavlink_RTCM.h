#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_RTCM_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_RTCM_H_

#include "../../Protocol/_Mavlink.h"
#include "../../Navigation/_RTCMcast.h"

namespace kai
{

	class _APmavlink_RTCM : public _RTCMcast
	{
	public:
		_APmavlink_RTCM();
		~_APmavlink_RTCM();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		virtual bool writeMavlink(const RTCM_MSG &msg);
		virtual void sendMsg(void);
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
		mavlink_gps_rtcm_data_t m_D;
	};

}
#endif
