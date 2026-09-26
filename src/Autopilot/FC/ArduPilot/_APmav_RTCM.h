#ifndef OpenKAI_src_Autopilot_FC_ArduPilot__APmav_RTCM_H_
#define OpenKAI_src_Autopilot_FC_ArduPilot__APmav_RTCM_H_

#include "../../../Protocol/_Mavlink.h"
#include "../../../Navigation/_RTCMcast.h"

#define GPS_DATA_FRAG_N 180

namespace kai
{

	class _APmav_RTCM : public _RTCMcast
	{
	public:
		_APmav_RTCM();
		~_APmav_RTCM();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		virtual bool writeMavlink(RTCM_MSG *pM);
		virtual void writeMsg(void);
		virtual void handleMsg(const RTCM_MSG &msg);

	protected:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_APmav_RTCM *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_APmav_RTCM *)This)->updateR();
			return NULL;
		}

	protected:
		_Mavlink *m_pMav = nullptr;

		uint8_t m_iSeq = 0;
	};

}
#endif
