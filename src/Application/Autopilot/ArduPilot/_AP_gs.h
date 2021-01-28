#ifndef OpenKAI_src_Application_Autopilot_ArduPilot__AP_gs_H_
#define OpenKAI_src_Application_Autopilot_ArduPilot__AP_gs_H_

#include "_AP_base.h"
#include "../../../GCS/_GCSbase.h"

namespace kai
{

class _AP_gs: public _GCSbase
{
public:
	_AP_gs();
	~_AP_gs();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);
    
    void getState(int* pState, GCS_STATE* pGstate);
    void landingReady(bool bReady);
    void takeoffReady(bool bReady);
    
protected:
    void updateGS (void);
	static void* getUpdateThread(void* This)
	{
		((_AP_gs*) This)->update();
		return NULL;
	}

public:
    _AP_base* m_pAP;
};

}
#endif
