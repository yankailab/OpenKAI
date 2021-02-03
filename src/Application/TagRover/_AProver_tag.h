#ifndef OpenKAI_src_Application_TagRover__AProver_tag_H_
#define OpenKAI_src_Application_TagRover__AProver_tag_H_

#include "../Autopilot/ArduPilot/_AP_base.h"
#include "../Autopilot/ArduPilot/_AProver_drive.h"
#include "../../Universe/_Universe.h"
#include "../../Utility/RC.h"

namespace kai
{

struct APROVER_TAG_MODE
{
	int8_t MANUAL;
	int8_t AUTO;

	bool bValid(void)
	{
		IF_F(MANUAL < 0);
		IF_F(AUTO < 0);

		return true;
	}
};

class _AProver_tag: public _StateBase
{
public:
	_AProver_tag();
	~_AProver_tag();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

protected:
	void updateMode(void);
	static void* getUpdate(void* This)
	{
		((_AProver_tag *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;
    _Drive*    m_pD;
    RC_CHANNEL m_rcMode;
    APROVER_TAG_MODE m_iMode;

};

}
#endif
