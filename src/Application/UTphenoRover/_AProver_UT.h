#ifndef OpenKAI_src_Application_UTphenoRovere__AProver_UT_H_
#define OpenKAI_src_Application_UTphenoRovere__AProver_UT_H_

#include "../Autopilot/ArduPilot/_AP_base.h"
#include "../Autopilot/ArduPilot/_AProver_drive.h"
#include "../../Universe/_Universe.h"
#include "../../Utility/RC.h"

namespace kai
{

struct APROVER_UT_MODE
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
};

class _AProver_UT: public _StateBase
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
	void updateMode(void);
	static void* getUpdate(void* This)
	{
		((_AProver_UT *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;
    _Drive*    m_pD;
    RC_CHANNEL m_rcMode;
    APROVER_UT_MODE m_iMode;

};

}
#endif
