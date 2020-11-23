
#ifndef OpenKAI_src_Autopilot_AP__AProver_follow_H_
#define OpenKAI_src_Autopilot_AP__AProver_follow_H_

#include "_AP_base.h"
#include "_AProver_drive.h"
#include "../../../Universe/_Universe.h"
#include "../../../Utility/RC.h"
#include "../../../Control/PIDctrl.h"

namespace kai
{

struct APROVER_FOLLOW_MODE
{
	int8_t MANUAL;
	int8_t HYBRID;
	int8_t AUTO;

	bool bValid(void)
	{
		IF_F(MANUAL < 0);
		IF_F(HYBRID < 0);
		IF_F(AUTO < 0);

		return true;
	}
};

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
	void updateMode(void);
	void updateDrive(void);
    _Object* findTarget(void);
	static void* getUpdateThread(void* This)
	{
		(( _AProver_follow *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_AProver_drive* m_pD;
    _Universe* m_pU;
    PIDctrl* m_pXpid;
    
   	int         m_iClass;
	bool	    m_bTarget;
	vFloat3     m_vP;		//x,y:variable, screen coordinate of the object being followed, z:variable in mm unit
	vFloat3     m_vPtarget;	//x,y:constant, screen coordinate where the followed object should get to, z:variable in mm unit
	float       m_nSpeed;
	float	    m_yaw;
    
    RC_CHANNEL m_rcMode;
    APROVER_FOLLOW_MODE m_iMode;
    RC_CHANNEL m_rcDir;
    
};

}
#endif
