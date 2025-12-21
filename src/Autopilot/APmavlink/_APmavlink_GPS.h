#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_GPS_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_GPS_H_

#include "../../Filter/Median.h"
#include "../../Navigation/Coordinate.h"
#include "../../Navigation/_NavBase.h"
#include "_APmavlink_base.h"

namespace kai
{

	class _APmavlink_GPS : public _ModuleBase
	{
	public:
		_APmavlink_GPS();
		~_APmavlink_GPS();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		bool reset(void);

	protected:
		void updateGPS(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_GPS *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
		_NavBase *m_pSB;
		Coordinate m_GPS;

		double m_yaw;
		bool m_bYaw;
		int m_iRelayLED;

		vInt3 m_vAxisIdx;
		vFloat3 m_vAxisK;

		LL_POS m_llPos;
		UTM_POS m_utmPos;
		LL_POS m_llOrigin;
		UTM_POS m_utmOrigin;
		bool m_bUseApOrigin;

		mavlink_gps_input_t m_D;
	};

}
#endif
