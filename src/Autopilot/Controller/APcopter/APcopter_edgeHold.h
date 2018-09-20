#ifndef OpenKAI_src_Autopilot_Controller_APcopter_edgeHold_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_edgeHold_H_

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

class APcopter_edgeHold: public ActionBase
{
public:
	APcopter_edgeHold();
	~APcopter_edgeHold();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

public:
	APcopter_base* m_pAP;
	_DepthEdge* m_pDE;
	_DistSensorBase* m_pDB;
	GPS* m_pGPS;

	Median m_fX;
	Median m_fY;

	double m_zTop;
	vDouble4 m_vPos;
};

}
#endif
