#ifndef OpenKAI_src_Autopilot_AP__AProver_field_H_
#define OpenKAI_src_Autopilot_AP__AProver_field_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "_AP_base.h"

namespace kai
{

class _AProver_field: public _AutopilotBase
{
public:
	_AProver_field();
	~_AProver_field();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

	virtual void setSpeed(float v);
	virtual void setYaw(float v);
	virtual void setYawMode(bool bRelative);

protected:
	void findLineM(void);
	void findLineL(void);
	bool updateDrive(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_field *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;

	float	m_speed;
	float	m_yaw;
	float	m_yawMode;


	PIDctrl* m_pPIDhdg;
	_DetectorBase* m_pLineM;
	_DetectorBase* m_pLineL;

	vFloat2 m_vBorderLrange;
	float m_borderL;
	float m_borderLtarget;
	float m_kBorderLhdg;
	float m_dHdg;
	vFloat2 m_vdHdgRange;

	bool m_bLineM;
	float m_nSpeed;

	uint64_t m_tCamShutter;
	uint64_t m_tCamShutterStart;

	uint8_t m_iPinLEDtag;
	uint8_t m_iPinCamShutter;


};

}
#endif
