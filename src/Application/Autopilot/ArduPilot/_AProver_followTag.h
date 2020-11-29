
#ifndef OpenKAI_src_Autopilot_AP__AProver_followTag_H_
#define OpenKAI_src_Autopilot_AP__AProver_followTag_H_

#include "_AP_base.h"
#include "../../../Universe/_Universe.h"
#include "../../../Control/PIDctrl.h"
#include "../../../Control/Drive.h"
#include "../../../Utility/util.h"

namespace kai
{

class _AProver_followTag: public _MissionBase
{
public:
	_AProver_followTag();
	~_AProver_followTag();

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
		(( _AProver_followTag *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	Drive* m_pD;
    _Universe* m_pU;
    PIDctrl* m_pPIDtag;
    PIDctrl* m_pPIDhdg;
    
    float   m_targetHdg;    //given by current compass and tag direction
    float   m_dHdg;
    
   	int         m_iClass;
	vFloat3     m_vP;		//x,y:variable, screen coordinate of the object being followed, z:variable in mm unit
	vFloat3     m_vPtarget;	//x,y:constant, screen coordinate where the followed object should get to, z:variable in mm unit

	float       m_nSpd;
    float       m_dir; //forward/backward
	float	    m_tagTargetHdg;
    
};

}
#endif
