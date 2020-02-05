#ifndef OpenKAI_src_Autopilot_AP__AP_takeoff_H_
#define OpenKAI_src_Autopilot_AP__AP_takeoff_H_

#include "../../../Base/common.h"
#include "../ArduPilot/_AP_base.h"

namespace kai
{

class _AP_takeoff: public _AutopilotBase
{
public:
	_AP_takeoff();
	~_AP_takeoff();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updateMission(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_takeoff*) This)->update();
		return NULL;
	}

public:
	_AP_base* m_pAP;

	int m_iWP;
};

}
#endif
