#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_mission_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_mission_H_

#include "../../../Base/common.h"
#include "_APcopter_base.h"

namespace kai
{

class _APcopter_mission: public _AutopilotBase
{
public:
	_APcopter_mission();
	~_APcopter_mission();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updateMission(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_mission*) This)->update();
		return NULL;
	}

public:
	_APcopter_base* m_pAP;

	int m_iWP;
};

}
#endif
