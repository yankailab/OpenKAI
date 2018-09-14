#ifndef OpenKAI_src_Autopilot_Controller_APcopter_edgeHold_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_edgeHold_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DepthEdge.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../../Filter/Median.h"
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
	APcopter_posCtrlRC* m_pPC;
	_DepthEdge* m_pDE;
	_DistSensorBase* m_pDB;

	Median m_fX;
	Median m_fY;

	vDouble4 m_vTarget;
	vDouble4 m_vPos;

};

}
#endif
