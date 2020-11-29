#ifndef OpenKAI_src_Autopilot_AP__AProver_KU_H_
#define OpenKAI_src_Autopilot_AP__AProver_KU_H_

#include "_AP_base.h"
#include "../../../Control/Drive.h"
#include "../../../Utility/RC.h"

namespace kai
{
    
struct APROVER_KU_MODE
{
	int8_t STANDBY;
	int8_t HYBRID;
	int8_t AUTO;

	bool bValid(void)
	{
		IF_F(STANDBY < 0);
		IF_F(HYBRID < 0);
		IF_F(AUTO < 0);

		return true;
	}
};

class _AProver_KU: public _MissionBase
{
public:
	_AProver_KU();
	~_AProver_KU();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updateMode(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_KU*) This)->update();
		return NULL;
	}

public:
	_AP_base* m_pAP;
    Drive*    m_pD;
    
    RC_CHANNEL m_rcMode;
    RC_CHANNEL m_rcDir;
    
    APROVER_KU_MODE m_iMode;
};

}
#endif
