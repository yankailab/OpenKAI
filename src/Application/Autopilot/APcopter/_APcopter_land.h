#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_land_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_land_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_follow.h"

namespace kai
{

class _APcopter_land: public _APcopter_follow
{
public:
	_APcopter_land();
	~_APcopter_land();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

	bool updateLand(void);
	virtual bool updateTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_land *) This)->update();
		return NULL;
	}

public:
	_APcopter_base*	m_pAP;
	_DetectorBase*	m_pIRlock;
};
}
#endif
