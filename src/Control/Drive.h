#ifndef OpenKAI_src_Control__Drive_H_
#define OpenKAI_src_Control__Drive_H_

#include "ControlBase.h"

namespace kai
{
    
struct DRIVE_MOTOR
{
    //config
    float m_kSpd;
    float m_kDir;
    float m_kStr;
    vFloat2 m_vSpd;    //speed range
	vFloat2 m_vStr;    //steering range
    
    //var
    float m_spd;
    float m_str;

    void init(void)
    {
        m_kSpd = 1.0;
        m_kDir = 1.0;
        m_kStr = 1.0;
        m_vSpd.init(-1.0,1.0);
        m_vStr.init(-1.0,1.0);
        
        m_spd = 0.0;
        m_str = 0.0;
    }
    
    void update(float nSpd, float nDir, float nStr)
    {
        float nD = nDir * m_kDir;
        m_spd = m_vSpd.constrain(nSpd * m_kSpd * nD);
        m_str = m_vStr.constrain(nStr * m_kStr * nD);
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

	virtual void setDrive(float nSpd, float nDir, float nStr); //speed, direction, steering
	virtual float getSpeed(int iM);
 	virtual float getSteering(int iM);
    
private:
	float	m_nSpd;
	float	m_nDir;
    float	m_nStr;
	
    vector<DRIVE_MOTOR> m_vM;

};

}
#endif
