
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_ZEDSLAM_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_ZEDSLAM_H_

#include "../../../Base/common.h"
#include "../../../Vision/_ZED.h"
#include "../../../Utility/util.h"
#include "../ActionBase.h"
#include "../APcopter/APcopter_base.h"

namespace kai
{

class APcopter_zedSLAM: public ActionBase
{
public:
	APcopter_zedSLAM();
	~APcopter_zedSLAM();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	void updateZEDtracking();

private:
	APcopter_base* m_pAP;
	_ZED* m_pZED;

	vDouble3 m_mT;
	vDouble3 m_mR;

};

}

#endif

