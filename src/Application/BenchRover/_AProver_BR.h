#ifndef OpenKAI_src_Autopilot_AP__AProver_BR_H_
#define OpenKAI_src_Autopilot_AP__AProver_BR_H_

#include "../Autopilot/ArduPilot/_AP_base.h"
#include "../../Control/Drive.h"
#include "../../Utility/RC.h"

namespace kai
{

struct APROVER_BR_MODE
{
	int8_t MANUAL;
	int8_t FORWARD;
	int8_t BACKWARD;

	bool bValid(void)
	{
		IF_F(MANUAL < 0);
		IF_F(FORWARD < 0);
		IF_F(BACKWARD < 0);

		return true;
	}
	
	bool assign(_MissionControl* pMC)
    {
        NULL_F(pMC);
        MANUAL = pMC->getMissionIdx("MANUAL");
        FORWARD = pMC->getMissionIdx("FORWARD");
        BACKWARD = pMC->getMissionIdx("BACKWARD");
        
        return bValid();
    }
};

class _AProver_BR: public _MissionBase
{
public:
	_AProver_BR();
	~_AProver_BR();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updateMode(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_BR*) This)->update();
		return NULL;
	}

public:
	_AP_base* m_pAP;
    Drive*    m_pD;
    
    RC_CHANNEL m_rcMode;
    APROVER_BR_MODE m_iMode;
};

}
#endif
