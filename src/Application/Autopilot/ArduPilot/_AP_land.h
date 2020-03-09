#ifndef OpenKAI_src_Autopilot_AP__AP_land_H_
#define OpenKAI_src_Autopilot_AP__AP_land_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Filter/FilterBase.h"
#include "../../_AutopilotBase.h"
#include "../ArduPilot/_AP_follow.h"

namespace kai
{

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
	virtual bool findTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_land *) This)->update();
		return NULL;
	}

public:
	Median m_filter;

	float m_altLandMode;
	float m_detSizeLandMode;
	vFloat4 m_vRoiDetDescent;
	float m_dTarget;
	float m_dHdg;
	float m_dzHdg;
	float m_detRdz;

};
}
#endif
