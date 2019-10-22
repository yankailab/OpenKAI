#ifndef OpenKAI_src_Autopilot_Rover_Rover_base_H_
#define OpenKAI_src_Autopilot_Rover_Rover_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../_AutopilotBase.h"
#include "_Rover_CMD.h"

namespace kai
{

struct ROVER_CTRL
{
	float m_hdg;
	float m_targetHdg;
	float m_nSpeed;		//-1.0 to 1.0
	float m_nTargetSpeed;

	void init(void)
	{
		m_hdg = -1.0;
		m_targetHdg = -1.0;
		m_nSpeed = 0.0;
		m_nTargetSpeed = 0.0;
	}
};

class _Rover_base: public _AutopilotBase
{
public:
	_Rover_base();
	~_Rover_base();

	bool init(void* pKiss);
	int	 check(void);
	void update(void);
	void draw(void);

public:
	_Rover_CMD* m_pCMD;
	_Mavlink* m_pMavlink;

	ROVER_CTRL m_ctrl;

};

}
#endif
