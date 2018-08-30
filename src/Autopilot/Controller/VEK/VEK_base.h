#ifndef OpenKAI_src_Autopilot_Controller_VEK_VEK_base_H_
#define OpenKAI_src_Autopilot_Controller_VEK_VEK_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Protocol/_Canbus.h"
#include "../../ActionBase.h"
#include "../../../Navigation/_GPS.h"

namespace kai
{

class VEK_base: public ActionBase
{
public:
	VEK_base();
	~VEK_base();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);

	void cmd(void);

public:
	_IOBase* m_pVEK;
	_IOBase* m_pCMD;

	int m_pwmU;
	int m_pwmN;
	int m_pwmL;
	double m_vL;
	double m_vR;
	double m_vForward;

	vDouble3 m_dT;
	vDouble3 m_dRot;

};

}
#endif
