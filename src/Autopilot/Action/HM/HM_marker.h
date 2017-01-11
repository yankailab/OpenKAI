#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMMARKER_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMMARKER_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Algorithm/Filter.h"
#include "../../../Navigation/_Obstacle.h"
#include "../../../Stream/_StreamBase.h"
#include "../../../AI/_DetectNet.h"
#include "../ActionBase.h"
#include "HM_base.h"
#include "../../../AI/_AIbase.h"


namespace kai
{

class HM_marker: public ActionBase
{
public:
	HM_marker();
	~HM_marker();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:

#ifdef USE_TENSORRT
	_DetectNet* m_pDetector;
#elif defined USE_SSD
	_SSD* m_pDetector;
#endif
	int m_targetClass;

	HM_base* m_pHM;
	_StreamBase* m_pStream;

	double	m_speedP;
	double	m_steerP;

	OBJECT* m_pTarget;
	kai::Filter* m_pTargetX;
	kai::Filter* m_pTargetY;
	kai::Filter* m_pTargetW;
	kai::Filter* m_pTargetH;
	double m_targetDist;
};

}

#endif

