#ifndef OpenKAI_src_Autopilot_Controller_APcopter_posCtrlBase_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_posCtrlBase_H_

#include "../../../Base/common.h"
#include "../../../Control/PIDctrl.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

#define CTRL_PITCH 0
#define CTRL_ROLL 1
#define CTRL_ALT 2
#define CTRL_YAW 3
#define N_CTRL 4

class APcopter_posCtrlBase: public ActionBase
{
public:
	APcopter_posCtrlBase();
	~APcopter_posCtrlBase();

	virtual bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual int check(void);

	virtual void setTargetPos(vDouble4& vT);
	virtual void setPos(vDouble4& vP);
	virtual void clear(void);

public:
	APcopter_base* m_pAP;
	vDouble4 m_vTarget;
	vDouble4 m_vPos;
};

}
#endif
