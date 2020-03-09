#ifndef OpenKAI_src_Autopilot_AP__AP_goto_H_
#define OpenKAI_src_Autopilot_AP__AP_goto_H_

#include "../../../Base/common.h"
#include "../../Mission/Waypoint.h"
#include "../ArduPilot/_AP_posCtrl.h"

namespace kai
{

class _AP_goto: public _AP_posCtrl
{
public:
	_AP_goto();
	~_AP_goto();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

private:
	bool updateGoto(void);
	bool findTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_goto *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;
	_AP_base*	m_pAPtarget;
	bool		m_bTarget;

	AP_MOUNT m_apMount;

};

}
#endif
