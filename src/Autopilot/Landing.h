
#ifndef OPENKAI_SRC_AUTOPILOT_LANDING_H_
#define OPENKAI_SRC_AUTOPILOT_LANDING_H_

#include "../Base/common.h"
#include "commonAutopilot.h"
#include "ActionBase.h"

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

class Landing: public ActionBase
{
public:
	Landing();
	~Landing();

	bool init(Config* pConfig, AUTOPILOT_CONTROL* pAC);
	void update(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);

public:
	void landingAtAprilTags(void);
	void landingAtBullseye(void);

private:
	//Detectors
	_Cascade*		m_pFD;
	_ROITracker* 	m_pROITracker;
	_Bullseye*		m_pMD;

	_AprilTags*		m_pAT;
	APRIL_TAG		m_pATags[NUM_PER_TAG];
	int				m_pATagsLandingTarget[NUM_PER_TAG];
	int				m_numATagsLandingTarget;

	LANDING_TARGET	m_landingTarget;



};

}

#endif

