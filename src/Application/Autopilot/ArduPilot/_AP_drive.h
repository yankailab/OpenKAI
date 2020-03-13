#ifndef OpenKAI_src_Autopilot_AP__AP_drive_H_
#define OpenKAI_src_Autopilot_AP__AP_drive_H_

#include "_AP_base.h"

namespace kai
{

class _AP_drive: public _AutopilotBase
{
public:
	_AP_drive();
	~_AP_drive();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

	virtual void setSpeed(float v);
	virtual void setYaw(float v);
	virtual void setYawMode(bool bRelative);

protected:
	bool updateDrive(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_drive *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;

	float	m_speed;
	float	m_yaw;
	float	m_yawMode;

};

}
#endif
