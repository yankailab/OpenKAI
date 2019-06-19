#ifndef OpenKAI_src_Autopilot_Controller_APcopter_land_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_land_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../ActionBase.h"
#include "APcopter_target.h"

namespace kai
{

class APcopter_land: public APcopter_target
{
public:
	APcopter_land();
	~APcopter_land();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

public:
	double m_altLanded;
	double m_apAlt;
};
}
#endif
