#ifndef OPENKAI_SRC_AUTOPILOT_HM_H_
#define OPENKAI_SRC_AUTOPILOT_HM_H_

#include "../Base/common.h"
#include "ActionBase.h"
#include "../UI/UI.h"
#ifdef USE_TENSORRT
#include "../AI/_DetectNet.h"
#endif
#ifdef USE_SSD
#include "../AI/_SSD.h"
#endif


namespace kai
{

class HM: public ActionBase
{
public:
	HM();
	~HM();

	bool init(Config* pConfig, AUTOPILOT_CONTROL* pAC);
	void update(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);

private:
	void updateMotion(void);
	void updateCAN(void);

	//Detectors
#ifdef USE_TENSORRT
	_DetectNet* m_pDN;
#endif
#ifdef USE_SSD
	_SSD* m_pSSD;
#endif

	//TODO: add PID

	double m_maxSpeed;
	int m_motorPwmL;
	int m_motorPwmR;
	int m_motorPwmW;
	bool m_bSpeaker;

	uint8_t m_ctrlB0;
	uint8_t m_ctrlB1;

};

}

#endif

