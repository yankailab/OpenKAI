
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_ZEDSLAM_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_ZEDSLAM_H_

#include "../../../Base/common.h"
#include "../../../Vision/_ZED.h"
#include "../../../Utility/util.h"
#include "../ActionBase.h"
#include "APMcopter_base.h"

namespace kai
{

class APMcopter_zedSLAM: public ActionBase
{
public:
	APMcopter_zedSLAM();
	~APMcopter_zedSLAM();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	void updateZEDtracking();

private:
	APMcopter_base* m_pAPM;
	_ZED* m_pZED;

	vDouble3 m_mT;
	vDouble3 m_mR;

};

}

#endif

