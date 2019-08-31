#ifndef OpenKAI_src_Autopilot_Controller_Rover_Rover_move_H_
#define OpenKAI_src_Autopilot_Controller_Rover_Rover_move_H_

#include "../../../../Detector/_DetectorBase.h"
#include "Rover_base.h"

namespace kai
{

class Rover_move: public ActionBase
{
public:
	Rover_move();
	~Rover_move();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

	void findLineM(void);
	void findLineL(void);

public:
	Rover_base* m_pR;
	_RoverCMD* m_pCMD;
	PIDctrl* m_pPIDhdg;
	_DetectorBase* m_pLineM;
	_DetectorBase* m_pLineL;

	vFloat2 m_vBorderLrange;
	float m_borderL;
	float m_borderLtarget;
	float m_kBorderLhdg;
	float m_dHdg;
	vFloat2 m_vdHdgRange;

	bool m_bLineM;
	float m_nSpeed;

	uint64_t m_tCamShutter;
	uint64_t m_tCamShutterStart;

	uint8_t m_iPinLEDtag;
	uint8_t m_iPinCamShutter;
};

}
#endif
