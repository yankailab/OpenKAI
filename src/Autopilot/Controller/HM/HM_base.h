#ifndef OpenKAI_src_Autopilot_Controller_HM_base_H_
#define OpenKAI_src_Autopilot_Controller_HM_base_H_

#include "../../../Base/common.h"
#include "../../../IO/_TCP.h"
#include "../../../Protocol/_Canbus.h"
#include "../../ActionBase.h"

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
	void updateGPS(void);

public:
	_Canbus* m_pCAN;
	_TCP* m_pCMD;
	string 	m_strCMD;

	int m_maxRpmT;
	int m_maxRpmW;
	int m_rpmL;
	int m_rpmR;
	int m_defaultRpmT;
	int m_motorRpmW;
	bool m_bSpeaker;
	bool m_bMute;
	unsigned long m_canAddrStation;

	uint8_t m_ctrlB0;
	uint8_t m_ctrlB1;

	double	m_wheelR;
	double	m_treadW;

	uint8_t m_pinLEDl;
	uint8_t m_pinLEDm;
	uint8_t m_pinLEDr;

};

}
#endif

