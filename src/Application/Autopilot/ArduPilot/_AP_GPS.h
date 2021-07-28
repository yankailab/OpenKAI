#ifndef OpenKAI_src_Autopilot_AP__AP_GPS_H_
#define OpenKAI_src_Autopilot_AP__AP_GPS_H_

#include "../../../Filter/Median.h"
#include "../../../Navigation/Coordinate.h"
#include "../../../Navigation/_NavBase.h"
#include "../ArduPilot/_AP_base.h"

#ifdef USE_REALSENSE

namespace kai
{

class _AP_GPS: public _StateBase
{
public:
	_AP_GPS();
	~_AP_GPS();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void console(void* pConsole);

	bool reset(void);

protected:
	void updateGPS(void);
	static void* getUpdate(void* This)
	{
		((_AP_GPS *) This)->update();
		return NULL;
	}

public:
	_AP_base* m_pAP;
	_NavBase* m_pSB;
	Coordinate m_GPS;

	double	m_yaw;
	bool	m_bYaw;
	int		m_iRelayLED;

	vInt3 m_vAxisIdx;
	vFloat3 m_vAxisK;

	LL_POS m_llPos;
	UTM_POS m_utmPos;
	LL_POS m_llOrigin;
	UTM_POS m_utmOrigin;
	bool	m_bUseApOrigin;

	mavlink_gps_input_t m_D;

};

}
#endif
#endif
