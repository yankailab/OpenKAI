
#ifndef OpenKAI_src_Application_KUphenoRover__AProver_KUfollowTag_H_
#define OpenKAI_src_Application_KUphenoRover__AProver_KUfollowTag_H_

#include "../Autopilot/ArduPilot/_AP_base.h"
#include "../../Universe/_Universe.h"
#include "../../Control/PID.h"
#include "../../Control/_Drive.h"
#include "../../Utility/util.h"

namespace kai
{

class _AProver_KUfollowTag: public _StateBase
{
public:
	_AProver_KUfollowTag();
	~_AProver_KUfollowTag();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	int check(void);
	void draw(void);

private:
	void updateFollow (void);
    _Object* findTarget(void);
	static void* getUpdateThread(void* This)
	{
		(( _AProver_KUfollowTag *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_Drive* m_pD;
    _Universe* m_pU;
    PID* m_pPIDtagX;
    PID* m_pPIDtagHdg;
    
    float m_errX;
    float m_errHdg;

	float   m_nSpd;
    float   m_nStr;
	float   m_tagTargetX;    
   	int     m_iTagStop;
    
};

}
#endif
