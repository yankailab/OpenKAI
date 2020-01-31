#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_land_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_land_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_follow.h"

namespace kai
{

enum LAND_TARGET_TYPE
{
	landTarget_unknown,
	landTarget_det,
	landTarget_IR,
};

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

	bool updateTarget(void);
	virtual bool findTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_land *) This)->update();
		return NULL;
	}

public:
	_DetectorBase*	m_pIRlock;

	float m_altLandMode;
	float m_dTarget;
	LAND_TARGET_TYPE m_targetType;

};
}
#endif
