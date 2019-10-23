#ifndef OpenKAI_src_Autopilot_Rover__Rover_avoid_H_
#define OpenKAI_src_Autopilot_Rover__Rover_avoid_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_avoid: public _AutopilotBase
{
public:
	_Rover_avoid();
	~_Rover_avoid();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updateAvoid(void);
	static void* getUpdateThread(void* This)
	{
		((_Rover_avoid*) This)->update();
		return NULL;
	}

public:
	_DepthVisionBase* m_pDV;

	ROVER_CTRL m_ctrl;
	float m_d;
	float m_minD;
	vFloat4 m_vRoi;
};

}
#endif
