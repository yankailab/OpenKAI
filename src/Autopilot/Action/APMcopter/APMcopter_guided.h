
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_GUIDED_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_GUIDED_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_Lightware_SF40.h"
#include "../ActionBase.h"
#include "APMcopter_base.h"

namespace kai
{

class APMcopter_guided: public ActionBase
{
public:
	APMcopter_guided();
	~APMcopter_guided();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	void updateAttitude(void);

private:
	APMcopter_base* m_pAPM;

	//sensor
	_Lightware_SF40* m_pSF40;




};

}

#endif

