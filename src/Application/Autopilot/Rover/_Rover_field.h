#ifndef OpenKAI_src_Autopilot_Rover__Rover_field_H_
#define OpenKAI_src_Autopilot_Rover__Rover_field_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_field: public _AutopilotBase
{
public:
	_Rover_field();
	~_Rover_field();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	void draw(void);

	void findLineM(void);
	void findLineL(void);

public:
	_Rover_base* m_pR;
	_Rover_CMD* m_pCMD;
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
