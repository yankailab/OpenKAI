#ifndef OpenKAI_src_Control__Drive_H_
#define OpenKAI_src_Control__Drive_H_

#include "ControlBase.h"

namespace kai
{
    
struct DRIVE_MOTOR
{
    //config
    float m_kSpd;           //velocity +/-
    float m_kStr;
    vFloat2 m_vSpdRange;    //speed range
	vFloat2 m_vStrRange;    //steering range
    
    //var
    float m_spd;
    float m_str;

    void init(void)
    {
        m_kSpd = 1.0;
        m_kStr = 1.0;
        m_vSpdRange.init(-1.0,1.0);
        m_vStrRange.init(-1.0,1.0);
        
        m_spd = 0.0;
        m_str = 0.0;
    }
    
    void update(float nSpd, float nStr)
    {
        m_spd = m_vSpdRange.constrain(nSpd * m_kSpd);
        m_str = m_vStrRange.constrain(nStr * m_kStr);
    }
};

class Drive: public ControlBase
{
public:
	Drive();
	~Drive();

	virtual	bool init(void* pKiss);
	virtual void update(void);
	virtual void draw(void);

	virtual void setSpeed(float nSpd);
    virtual void setDirection(float nDir);
    virtual void setSteering(float nStr);
	virtual float getSpeed(int iM);
 	virtual float getSteering(int iM);
    
private:
	float	m_nSpd;
    float   m_nDir;
    float	m_nStr;
	
    vector<DRIVE_MOTOR> m_vM;

};

}
#endif
