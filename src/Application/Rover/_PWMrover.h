#ifndef OpenKAI_src_Autopilot_Rover__PWMrover_H_
#define OpenKAI_src_Autopilot_Rover__PWMrover_H_

#include "../../Vision/_DepthVisionBase.h"
#include "../../Universe/_Universe.h"
#include "../../Protocol/_ProtocolBase.h"
#include "../../Autopilot/Drive/_Drive.h"

namespace kai
{

class _PWMrover: public _StateBase
{
public:
	_PWMrover();
	~_PWMrover();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void console(void* pConsole);

protected:
	bool updateDrive(void);
	static void* getUpdate(void* This)
	{
		((_PWMrover *) This)->update();
		return NULL;
	}

public:
	_DepthVisionBase* m_pDV;
	_Universe* m_pU;
	_ProtocolBase* m_pP;
    _Drive* m_pD;

	uint8_t m_iSpd;		// channel index
	uint8_t m_iSteer;

	float m_nSpd;
	float m_nSteer;

};

}
#endif
