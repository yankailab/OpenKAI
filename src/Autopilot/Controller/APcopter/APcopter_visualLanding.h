
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_VISUALLANDING_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_VISUALLANDING_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

struct LANDING_TARGET
{
	double m_angleX;
	double m_angleY;
	double m_orientX;
	double m_orientY;
	uint64_t m_timeStamp;
	uint64_t m_timeOut;
	double m_minSize;
	double m_maxSize;
	vInt2  m_targetPos;
	bool	m_bLocked;

	void init(void)
	{
		m_angleX = 0;
		m_angleY = 0;
		m_orientX = 1.0;
		m_orientY = 1.0;
		m_timeStamp = 0;
		m_timeOut = 100000;
		m_minSize = 0.0;
		m_maxSize = 1.0;
		m_targetPos.init();
		m_bLocked = false;
	}
};

class APcopter_visualLanding: public ActionBase
{
public:
	APcopter_visualLanding();
	~APcopter_visualLanding();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	void landing(void);

private:
	APcopter_base* m_pAP;
	_DetectorBase* m_pAI;

	LANDING_TARGET	m_target;

};

}

#endif

