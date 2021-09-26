#ifndef OpenKAI_src_Autopilot_AP__AP_land_H_
#define OpenKAI_src_Autopilot_AP__AP_land_H_
#include "../../Detector/_DetectorBase.h"
#include "_AP_follow.h"

namespace kai
{

class _AP_land: public _AP_follow
{
public:
	_AP_land();
	~_AP_land();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void cvDraw(void* pWindow);
	void console(void* pConsole);

protected:
	bool updateTarget(void);
	virtual bool findTarget(void);
	static void* getUpdate(void* This)
	{
		(( _AP_land *) This)->update();
		return NULL;
	}

public:
    float m_zrK;
	float m_dTarget;	//dist to target
	float m_dTargetComplete;
	float m_altComplete;
};
}
#endif
