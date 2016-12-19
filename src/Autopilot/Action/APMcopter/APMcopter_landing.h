
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_LANDING_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_LANDING_H_

#include "../../../Base/common.h"
#include "../../../Detector/_Bullseye.h"
#include "../../../Detector/_AprilTags.h"
#include "../../../Tracker/_ROITracker.h"
#include "../ActionBase.h"
#include "APMcopter_base.h"

#ifdef USE_OPENCV3

namespace kai
{

struct LANDING_TARGET
{
	double m_angleX;
	double m_angleY;
	double m_orientX;
	double m_orientY;
	uint64_t m_ROIstarted;
	uint64_t m_ROItimeLimit;

};

class APMcopter_landing: public ActionBase
{
public:
	APMcopter_landing();
	~APMcopter_landing();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	void landingAtAprilTags(void);
	void landingAtBullseye(void);

private:
	APMcopter_base* m_pAPM;

	//Detectors
	_ROITracker* 	m_pROITracker;
	_Bullseye*		m_pMD;

	_AprilTags*	m_pAT;
	APRIL_TAG	m_pATags[NUM_PER_TAG];
	int		m_pATagsLandingTarget[NUM_PER_TAG];
	int		m_numATagsLandingTarget;

	LANDING_TARGET	m_landingTarget;

};

}

#endif
#endif

