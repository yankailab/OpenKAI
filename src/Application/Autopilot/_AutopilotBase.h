
#ifndef OpenKAI_src_Autopilot_AutopilotBase_H_
#define OpenKAI_src_Autopilot_AutopilotBase_H_

#include "../../Base/_ThreadBase.h"
#include "../Mission/_MissionControl.h"

namespace kai
{

class _AutopilotBase: public _ThreadBase
{
public:
	_AutopilotBase();
	~_AutopilotBase();

	virtual bool init(void* pKiss);
	virtual void update(void);
	virtual void draw(void);
	virtual bool bActive(void);
	virtual bool bMissionChanged(void);
	virtual int check(void);

	virtual void drawActive(void);

public:
	_MissionControl* m_pMC;
	std::vector<int> m_vActiveMission;
	int m_iLastMission;
	bool m_bMissionChanged;

	int64_t m_tStamp;
	int64_t m_dTime;

	_AutopilotBase* m_pAB;
	void* m_pCtrl;
};

}
#endif
