#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HM_BASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HM_BASE_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Canbus.h"
#include "../../../IO/TCP.h"
#include "../../../Navigation/_Path.h"
#include "../ActionBase.h"

namespace kai
{

class HM_base: public ActionBase
{
public:
	HM_base();
	~HM_base();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	void updateCAN(void);
	bool draw(void);

	void cmd(void);

public:
	_Canbus* m_pCAN;
	IO* m_pCMD;
	string m_strCMD;
	_Path*	m_pPath;

	int m_maxSpeedT;
	int m_maxSpeedW;
	int m_motorPwmL;
	int m_motorPwmR;
	int m_motorPwmW;
	int m_speedP;
	bool m_bSpeaker;
	bool m_bInStation;

	uint8_t m_ctrlB0;
	uint8_t m_ctrlB1;

};

}

#endif

