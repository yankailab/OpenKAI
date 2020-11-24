
#ifndef OpenKAI_src_Autopilot_AP__AProver_follow_H_
#define OpenKAI_src_Autopilot_AP__AProver_follow_H_

#include "_AP_base.h"
#include "../../../Universe/_Universe.h"
#include "../../../Control/PIDctrl.h"
#include "../../../Control/Drive.h"

namespace kai
{

class _AProver_follow: public _MissionBase
{
public:
	_AProver_follow();
	~_AProver_follow();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	int check(void);
	void draw(void);

private:
	void updateDrive(void);
    _Object* findTarget(void);
	static void* getUpdateThread(void* This)
	{
		(( _AProver_follow *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	Drive* m_pD;
    _Universe* m_pU;
    PIDctrl* m_pXpid;
        
   	int         m_iClass;
	vFloat3     m_vP;		//x,y:variable, screen coordinate of the object being followed, z:variable in mm unit
	vFloat3     m_vPtarget;	//x,y:constant, screen coordinate where the followed object should get to, z:variable in mm unit

	float       m_nSpd;
    float       m_dir; //forward/backward
	float	    m_yaw;
    
};

}
#endif
