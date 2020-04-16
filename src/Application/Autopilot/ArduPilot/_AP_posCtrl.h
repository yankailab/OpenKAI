#ifndef OpenKAI_src_Autopilot_AP__AP_posCtrl_H_
#define OpenKAI_src_Autopilot_AP__AP_posCtrl_H_

#include "../../../Base/common.h"
#include "../../../Control/PIDctrl.h"
#include "../ArduPilot/_AP_base.h"

namespace kai
{

class _AP_posCtrl: public _AutopilotBase
{
public:
	_AP_posCtrl();
	~_AP_posCtrl();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void update(void);
	virtual void draw(void);
	virtual int check(void);

	void clear(void);
	void releaseCtrl(void);
	void setPosLocal(void);
	void setPosGlobal(void);

private:
	static void* getUpdateThread(void* This)
	{
		((_AP_posCtrl *) This)->update();
		return NULL;
	}

public:
	_AP_base* m_pAP;

	mavlink_set_position_target_local_ned_t m_sptLocal;
	mavlink_set_position_target_global_int_t m_sptGlobal;

	//roll, pitch, alt, yaw
	vFloat4 m_vP;		//variable, screen coordinate of the object being followed in roll, pitch, alt, yaw order
	vFloat4 m_vTargetP;	//constant, screen coordinate where the followed object should get to
	vDouble4 m_vTargetGlobal;	//lat, lon, alt, hdg

	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pAlt;

	bool	m_bYaw;

};

}
#endif
