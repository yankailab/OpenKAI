#ifndef OpenKAI_src_Autopilot_AP__AP_CETCUS_H_
#define OpenKAI_src_Autopilot_AP__AP_CETCUS_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_CETCUS.h"
#include "../ArduPilot/_AP_base.h"

namespace kai
{

class _AP_CETCUS: public _AutopilotBase
{
public:
	_AP_CETCUS();
	~_AP_CETCUS();

	bool init(void* pKiss);
	int check(void);
	bool start(void);
	void update(void);
	void draw(void);

private:
	bool updateCETCUS(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_CETCUS *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_CETCUS* m_pC;

};

}
#endif
