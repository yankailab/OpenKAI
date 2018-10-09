#ifndef OpenKAI_src_Autopilot_Controller_APcopter_slam_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_slam_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DepthEdge.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../../Filter/Median.h"
#include "../../../Navigation/GPS.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrlRC.h"

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

public:
	APcopter_base* m_pAP;
	_DepthEdge* m_pDE;
	_DistSensorBase* m_pDB;
	GPS m_GPS;

	Median m_fX;
	Median m_fY;

	double m_zTop;
	vDouble4 m_vPos;
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
