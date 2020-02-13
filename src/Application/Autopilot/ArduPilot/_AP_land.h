#ifndef OpenKAI_src_Autopilot_AP__AP_land_H_
#define OpenKAI_src_Autopilot_AP__AP_land_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Filter/FilterBase.h"
#include "../../_AutopilotBase.h"
#include "../ArduPilot/_AP_follow.h"

namespace kai
{

enum LAND_TARGET_TYPE
{
	landTarget_unknown,
	landTarget_det,
	landTarget_IR,
	landTarget_global,
};

class _AP_land: public _AP_follow
{
public:
	_AP_land();
	~_AP_land();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

	bool updateTarget(void);
	virtual bool findTargetLocal(void);
	virtual bool findTargetGlobal(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_land *) This)->update();
		return NULL;
	}

public:
	_AP_base* m_pAPtarget;
	_DetectorBase*	m_pIRlock;
	Median m_filter;

	float m_altLandMode;
	float m_detSizeLandMode;
	float m_dTarget;
	float m_dHdg;
	float m_dzHdg;
	LAND_TARGET_TYPE m_targetType;

};
}
#endif
