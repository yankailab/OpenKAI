#ifndef OpenKAI_src_Autopilot_Controller_APcopter_slam_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_slam_H_

#include "../../../Base/common.h"
#include "../../../Filter/Median.h"
#include "../../../Navigation/GPS.h"
#include "../../../IO/_IOBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define MG_BUF_N 32
#define MG_CMD_START 0xff
#define MG_CMD_POS 0
#define MG_CMD_ATTITUDE 1
#define MG_CMD_RAW_IMU 2

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
	bool console(int& iY);
	int check(void);

	void updatePos(void);
	void sendState(void);

public:
	APcopter_base* m_pAP;
	_IOBase* m_pIOr;
	_IOBase* m_pIOw;
	GPS m_GPS;

	Median m_fX;
	Median m_fY;
	Median m_fHdg;

	UTM_POS m_utmPos;
	LL_POS m_llGPSorigin;
	UTM_POS m_utmGPSorigin;

	vDouble3 m_vSlamPos;
	vDouble3 m_vVelo;
	double	m_yawOffset;

	uint8_t	m_pCmd[MG_BUF_N];
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
2 | 0x00 MG_CMD_POS
3~4 | int16  Coordinate X * 1000
5~6 | int16  Coordinate Y * 1000
7~8 | int16  Coordinate Z * 1000
9~10 | int16  Heading in Degree * 1000
11 | uint8 Confidence 0 to 100, (set to 255 if confidence is not provided)

Byte | Content
1 | 0xFF packet start mark
2 | 0x01 MG_CMD_ATTITUDE
3~4 | int16  Roll in rad * 1000 (-pi ~ pi)
5~6 | int16  Pitch in rad * 1000 (-pi ~ pi)
7~8 | int16  Yaw (Heading) in rad * 1000 (0 ~ 2pi, 0 is North)

Byte | Content
1 | 0xFF packet start mark
2 | 0x02 MG_CMD_RAW_IMU
3~4 | int16  xAcc
5~6 | int16  yAcc
7~8 | int16  zAcc
9~10 | int16  xGyro
11~12 | int16  yGyro
13~14 | int16  zGyro
15~16 | int16  xMag in milligauss
17~18 | int16  yMag in milligauss
19~20 | int16  zMag in milligause
 */
