#ifndef OpenKAI_src_Mission__MissionBase_H_
#define OpenKAI_src_Mission__MissionBase_H_

#include "_MissionControl.h"

namespace kai
{

class _MissionBase: public _ThreadBase
{
public:
	_MissionBase();
	~_MissionBase();

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
};

}
#endif
