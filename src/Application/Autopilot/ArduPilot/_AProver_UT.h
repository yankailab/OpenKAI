#ifndef OpenKAI_src_Autopilot_AP__AProver_UT_H_
#define OpenKAI_src_Autopilot_AP__AProver_UT_H_

#include "_AP_base.h"
#include "_AProver_drive.h"
#include "../../../Universe/_Universe.h"
#include "../../../Utility/RC.h"

namespace kai
{

struct APROVER_UT_MODE
{
	int8_t STANDBY;
	int8_t FORWARD;
	int8_t BACKWARD;

	bool bValid(void)
	{
		IF_F(STANDBY < 0);
		IF_F(FORWARD < 0);
		IF_F(BACKWARD < 0);

		return true;
	}
};

class _AProver_UT: public _MissionBase
{
public:
	_AProver_UT();
	~_AProver_UT();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

protected:
	void findDivider(void);
	void findTag(void);
	void updateMode(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_UT *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;
    _Universe* m_pUdiv;
    _Universe* m_pUtag;

    //divider
	bool m_bDivider;

    //tag
    int         m_iClass;
	bool	    m_bTarget;
	float       m_nSpeed;
	float	    m_yaw;

    //input
    RC_CHANNEL m_rcDir;
    RC_CHANNEL m_rcMode;
    APROVER_UT_MODE m_iMode;

    //indicator
	uint8_t m_iPinLED;
	uint8_t m_iPinShutter;

};

}
#endif
