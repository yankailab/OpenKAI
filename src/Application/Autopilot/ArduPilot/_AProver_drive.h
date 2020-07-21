#ifndef OpenKAI_src_Autopilot_AP__AProver_drive_H_
#define OpenKAI_src_Autopilot_AP__AProver_drive_H_

#include "_AP_base.h"

namespace kai
{

class _AProver_drive: public _MissionBase
{
public:
	_AProver_drive();
	~_AProver_drive();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

	virtual void setSpeed(float nSpeed);
	virtual void setYaw(float yaw);
	virtual void setYawMode(bool bRelative);

protected:
	bool updateDrive(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_drive *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;

	float	m_nSpeed;
	float	m_kSpeed;	// +/-1.0 forward/backward
	float	m_yaw;

	float	m_speed;
	float	m_yawMode;

	bool	m_bSetYawSpeed;
	bool	m_bRcChanOverride;
	float	m_pwmM;
	float	m_pwmD;
	uint16_t* m_pRcYaw;
	uint16_t* m_pRcThrottle;
	mavlink_rc_channels_override_t m_rcOverride;

};

}
#endif
