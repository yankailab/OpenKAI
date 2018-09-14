#ifndef OpenKAI_src_Autopilot_Controller_APcopter_mission_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_mission_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Control/PIDctrl.h"
#include "../../ActionBase.h"

namespace kai
{

class APcopter_mission: public ActionBase
{
public:
	APcopter_mission();
	~APcopter_mission();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	int check(void);

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;
	uint32_t m_flightMode;

	int m_freqAtti;
	int m_freqGlobalPos;
	int m_freqHeartbeat;
	int m_freqRC;

};

}
#endif
