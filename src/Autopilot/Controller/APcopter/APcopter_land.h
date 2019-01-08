#ifndef OpenKAI_src_Autopilot_Controller_APcopter_land_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_land_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrl.h"

namespace kai
{

class APcopter_land: public ActionBase
{
public:
	APcopter_land();
	~APcopter_land();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

	void updateGimbal(void);

public:
	APcopter_base* m_pAP;
	APcopter_posCtrl* m_pPC;
	_ObjectBase* m_pDet;
	OBJECT m_tO;

	bool	 m_bGimbal;
	vDouble3 m_vGimbal;
	mavlink_mount_control_t m_mountControl;
	mavlink_mount_configure_t m_mountConfig;
};

}
#endif
