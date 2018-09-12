#ifndef OpenKAI_src_Autopilot_Controller_APcopter_posCtrlRC_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_posCtrlRC_H_

#include "../../../Base/common.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_posCtrlRC: public ActionBase
{
public:
	APcopter_posCtrlRC();
	~APcopter_posCtrlRC();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	void setTargetPos(vDouble3& vT);
	void setPos(vDouble3& vT);

public:
	APcopter_base* m_pAP;

	vDouble3 m_vTarget;
	vDouble3 m_vPos;
	uint32_t m_iModeEnable;

	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pYaw;
	PIDctrl* m_pAlt;

	uint16_t m_pwmLow;
	uint16_t m_pwmMidR;
	uint16_t m_pwmMidP;
	uint16_t m_pwmMidY;
	uint16_t m_pwmMidA;
	uint16_t m_pwmHigh;

};

}
#endif
