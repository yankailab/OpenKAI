#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_dynamicLanding_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_dynamicLanding_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_base.h"

namespace kai
{

class _APcopter_dynamicLanding: public _AutopilotBase
{
public:
	_APcopter_dynamicLanding();
	~_APcopter_dynamicLanding();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);
	void releaseCtrl(void);

private:
	void updateTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_dynamicLanding *) This)->update();
		return NULL;
	}

public:
	_APcopter_base*	m_pAP;
	_DetectorBase*	m_pIRlock;
	_DetectorBase*	m_pAruco;

	vFloat3 m_vSpeed;	//NED, +,-,-
	OBJECT m_tO;
	float m_degLand;

	mavlink_set_position_target_local_ned_t m_spt;

};
}
#endif
