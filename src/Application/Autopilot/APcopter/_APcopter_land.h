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
	int check(void);
	void update(void);
	void draw(void);

public:
	double m_altLanded;
	double m_apAlt;
};
}
#endif
