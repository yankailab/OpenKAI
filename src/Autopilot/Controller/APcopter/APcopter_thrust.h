
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_thrust_H_

#include "../../../Base/common.h"
#include "../../../SLAM/_SlamBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_thrust: public ActionBase
{
public:
	APcopter_thrust();
	~APcopter_thrust();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	void updateMavlink(void);

	APcopter_base* m_pAP;
	_SlamBase* m_pSB;

	vDouble3 m_pTarget;

};

}
#endif
