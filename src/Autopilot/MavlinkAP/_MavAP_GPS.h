#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_GPS_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_GPS_H_

#include "../../Filter/Median.h"
#include "../../Navigation/Coordinate.h"
#include "../../Navigation/_NavBase.h"
#include "_MavAP_base.h"

namespace kai
{

	class _MavAP_GPS : public _ModuleBase
	{
	public:
		_MavAP_GPS();
		~_MavAP_GPS();

		int init(void *pKiss);
		int start(void);
		int check(void);
		void update(void);
		void console(void *pConsole);

		bool reset(void);

	protected:
		void updateGPS(void);
		static void *getUpdate(void *This)
		{
			((_MavAP_GPS *)This)->update();
			return NULL;
		}

	public:
		_MavAP_base *m_pAP;
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
