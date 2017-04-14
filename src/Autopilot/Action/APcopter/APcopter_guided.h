
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_GUIDED_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_GUIDED_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_Lightware_SF40.h"
#include "../ActionBase.h"
#include "../APcopter/APcopter_base.h"

namespace kai
{

class APcopter_guided: public ActionBase
{
public:
	APcopter_guided();
	~APcopter_guided();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	void updateAttitude(void);

private:
	APcopter_base* m_pAPM;
	uint32_t m_lastFlightMode;

	//sensor
	_Lightware_SF40* m_pSF40;




};

}

#endif

