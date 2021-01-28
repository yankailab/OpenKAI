
#ifndef OpenKAI_src_Application_UTphenoRover__AProver_UTfollowTag_H_
#define OpenKAI_src_Application_UTphenoRover__AProver_UTfollowTag_H_

#include "../Autopilot/ArduPilot/_AP_base.h"
#include "../../Universe/_Universe.h"
#include "../../Control/PID.h"
#include "../../Control/_Drive.h"
#include "../../Utility/util.h"

namespace kai
{
    
struct UTPR_MISSIOIN
{
	int8_t DRIVE;
	int8_t DIVIDER;
	int8_t EXTRACT;
	int8_t FOLLOW;
	int8_t RECOVER;

	bool bValid(void)
	{
		IF_F(DRIVE < 0);
		IF_F(DIVIDER < 0);
		IF_F(EXTRACT < 0);
		IF_F(FOLLOW < 0);
		IF_F(RECOVER < 0);

		return true;
	}
	
	bool assign(_MissionControl* pMC)
    {
        NULL_F(pMC);
        DRIVE = pMC->getMissionIdxByName ("DRIVE");
        DIVIDER = pMC->getMissionIdxByName ("DIVIDER");
        EXTRACT = pMC->getMissionIdxByName ("EXTRACT");
        FOLLOW = pMC->getMissionIdxByName ("FOLLOW");
        RECOVER = pMC->getMissionIdxByName ("RECOVER");
        
        return bValid();
    }
};


class _AProver_UTfollowTag: public _MissionBase
{
public:
	_AProver_UTfollowTag();
	~_AProver_UTfollowTag();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	int check(void);
	void draw(void);

private:
	void updateFollow (void);
	static void* getUpdateThread(void* This)
	{
		(( _AProver_UTfollowTag *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_Drive* m_pD;

    _Universe* m_pUside;
    PID* m_pPIDside;
    float m_errSide;
	float m_targetSide;
    vFloat2 m_vSide;
    
    _Universe* m_pUdivider;
    bool m_bDivider;
    
    _Universe* m_pUtag;
    int m_iTag;
    vFloat2  m_vTagY;
    vFloat2  m_vTagX;

	float   m_nSpd;
    float   m_nStr;
    
    UTPR_MISSIOIN m_iMission;
};

}
#endif
