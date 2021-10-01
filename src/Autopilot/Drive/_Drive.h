#ifndef OpenKAI_src_Autopilot_Drive__Drive_H_
#define OpenKAI_src_Autopilot_Drive__Drive_H_

#include "../../State/_StateBase.h"
#include "../../Actuator/_ActuatorBase.h"

namespace kai
{
    
struct DRIVE_MOTOR
{
    float m_kSpd;           //velocity +/-
    float m_kStr;
    float m_spd;
    
    _ActuatorBase* m_pActuator;
    uint8_t m_iAxis;

    void init(void)
    {
        m_kSpd = 1.0;
        m_kStr = 1.0;
        m_spd = 0.0;
        m_pActuator = NULL;
        m_iAxis = 0;
    }
    
    void update(float nSpd, float nStr)
    {
        m_spd = nSpd * m_kSpd + nStr * m_kStr;
        
        NULL_(m_pActuator);
        m_pActuator->setStarget(m_iAxis, m_spd);
    }
};

class _Drive: public _StateBase
{
public:
	_Drive();
	~_Drive();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void console(void* pConsole);

	virtual void setSpeed(float nSpd);
    virtual void setDirection(float nDir);
    virtual void setSteering(float nStr);

    virtual float getSpeed(void);
    virtual float getDirection(void);
    virtual float getSteering(void);

    virtual float getMotorSpeed(int iM);

private:
	virtual void update(void);
   	static void* getUpdate(void* This)
	{
		(( _Drive*) This)->update();
		return NULL;
	}

private:
	float	m_nSpd;
    float   m_nDir;
    float	m_nStr;
	
    vFloat2 m_vSpdRange;    //overall speed range
	vFloat2 m_vStrRange;    //overall steering range

	vector<DRIVE_MOTOR> m_vM;

};

}
#endif
