
#ifndef OpenKAI_src_Application_KUphenoRover__AProver_BRfollow_H_
#define OpenKAI_src_Application_KUphenoRover__AProver_BRfollow_H_

#include "../../Universe/_Universe.h"
#include "../../Control/PID.h"
#include "../../Control/_Drive.h"
#include "_AProver_BR.h"

namespace kai
{

class _AProver_BRfollow: public _StateBase
{
public:
	_AProver_BRfollow();
	~_AProver_BRfollow();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	int check(void);
	void console(void* pConsole);

private:
	void updateFollow (void);
    _Object* findTarget(void);
	static void* getUpdate(void* This)
	{
		(( _AProver_BRfollow *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_Drive* m_pD;
    PID* m_pPIDtagX;
    PID* m_pPIDtagHdg;
    _Universe* m_pU;
        
	float   m_tagTargetX;
    float   m_errX;
    float   m_errHdg;
   	int     m_iTagStop;
    int     m_iTag;
	float   m_nSpd;
    float   m_nStr;
    
    int     m_tStop;
    
};

}
#endif
