#ifndef OpenKAI_src_Actuator_Articulated__xArm_H_
#define OpenKAI_src_Actuator_Articulated__xArm_H_

#include "../_ActuatorBase.h"

#ifdef USE_XARM
#include <xarm/wrapper/xarm_api.h>

namespace kai
{

class _xArm: public _ActuatorBase
{
public:
	_xArm();
	~_xArm();

	virtual bool init(void *pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

    virtual bool power(bool bON);

protected:
    virtual void updatePos(void);
    virtual void gotoPos(vFloat3& vP);
	virtual vFloat3 getPtarget(void);
	virtual vFloat3 getP(void);
	virtual vFloat3 getAtarget(void);
	virtual vFloat3 getA(void);
    virtual	void readState(void);

	void update(void);
	static void* getUpdate(void *This)
	{
		((_xArm*) This)->update();
		return NULL;
	}

public:
    XArmAPI *m_pArm;
    
    string m_ip;
	int m_mode;
	int m_state;

};

}
#endif
#endif
