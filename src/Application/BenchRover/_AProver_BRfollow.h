
#ifndef OpenKAI_src_Application_KUphenoRover__AProver_BRfollow_H_
#define OpenKAI_src_Application_KUphenoRover__AProver_BRfollow_H_

#include "../../Universe/_Universe.h"
#include "../../Control/PID.h"
#include "../../Control/_Drive.h"
#include "_AProver_BR.h"

namespace kai
{

class _AProver_BRfollow: public _MissionBase
{
public:
	_AProver_BRfollow();
	~_AProver_BRfollow();

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
		(( _AProver_BRfollow *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_Drive* m_pD;
    PID* m_pPID;
    _Universe* m_pUpath;
    _Universe* m_pUtag;
        
	float   m_targetX;
    float   m_errX;
	float   m_nSpd;
    float   m_nStr;
   	int     m_iTagStop;
    
};

}
#endif
