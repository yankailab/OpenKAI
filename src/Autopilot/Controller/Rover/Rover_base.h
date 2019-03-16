#ifndef OpenKAI_src_Autopilot_Controller_Rover_Rover_base_H_
#define OpenKAI_src_Autopilot_Controller_Rover_Rover_base_H_

#include "../../../Base/common.h"
#include "../../ActionBase.h"
#include "_RoverCMD.h"

namespace kai
{

class Rover_base: public ActionBase
{
public:
	Rover_base();
	~Rover_base();

	bool init(void* pKiss);
	int	 check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

	void cmd(void);

public:
	_RoverCMD* m_pCMD;

	float m_hdg;	//-1.0 to 1.0
	float m_speed;	//-1.0 to 1.0

};

}
#endif
