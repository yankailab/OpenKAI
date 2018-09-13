#ifndef OpenKAI_src_Autopilot_Controller_APcopter_arucoFollow_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_arucoFollow_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrlRC.h"

namespace kai
{

class APcopter_arucoFollow: public ActionBase
{
public:
	APcopter_arucoFollow();
	~APcopter_arucoFollow();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	OBJECT* newFound(void);

public:
	APcopter_base* m_pAP;
	APcopter_posCtrlRC* m_pPC;
	_ObjectBase* m_pArUco;

	int 	 m_tag;
	double 	 m_angle;
	bool	 m_bTarget;

	vDouble4 m_vTarget;
	vDouble4 m_vPos;

};

}
#endif
