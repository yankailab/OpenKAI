#ifndef OpenKAI_src_Autopilot_Controller_APcopter_slam_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_slam_H_

#include "../../../Base/common.h"
#include "../../../Filter/Median.h"
#include "../../../Navigation/GPS.h"
#include "../../../IO/_IOBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define MG_PACKET_N 18

namespace kai
{

class APcopter_slam: public ActionBase
{
public:
	APcopter_slam();
	~APcopter_slam();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	void updatePos(void);

public:
	APcopter_base* m_pAP;
	_IOBase* m_pIO;
	GPS m_GPS;

	Median m_fX;
	Median m_fY;
	Median m_fHdg;

	double m_zTop;
	LL_POS m_vGPSorigin;
	vDouble3 m_vSlamOrigin;
	vDouble3 m_vSlamPos;
	uint32_t m_fModeOriginReset;
	double	m_yawOffset;

	uint8_t	m_pCmd[MG_PACKET_N];
	int m_iCmd;

	int m_gpsID;
	int m_iFixType;
	int m_nSat;
	double m_hdop;
	double m_vdop;

};

}
#endif

/*
Byte | Content
1 | 0xFF packet start mark
2~5 | int32  Coordinate X * 1000
6~9 | int32  Coordinate Y * 1000
10~13 | int32  Coordinate Z * 1000 (Not used at the moment)
14~17 | int32  Heading in Degree * 1000
18 | uint8 Confidence 0 to 100, (set to 255 if confidence is not provided)
 */
