#ifndef OpenKAI_src_Autopilot_Controller_APcopter_arucoFollow_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_arucoFollow_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

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
	void releaseRC(void);

public:
	APcopter_base* m_pAP;
	_ObjectBase* m_pArUco;
	_DepthVisionBase* m_pDV;

	int 	 m_tag;
	double 	 m_angle;
	bool	 m_bFollowing;

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
