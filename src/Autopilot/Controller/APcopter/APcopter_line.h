#ifndef OpenKAI_src_Autopilot_Controller_APcopter_line_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_line_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrl.h"

namespace kai
{

class APcopter_line: public ActionBase
{
public:
	APcopter_line();
	~APcopter_line();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual bool find(void);
	virtual bool bFound(void);

public:
	APcopter_base*		m_pAP;
	APcopter_posCtrl*	m_pPC;
	_DetectorBase*		m_pDet;
	_DistSensorBase*	m_pDS;

	OBJECT			m_tO;
	bool			m_bFound;

	//roll, pitch, alt, yaw
	vDouble4 		m_vTargetPos;
	vDouble4 		m_vMyPos;
	bool			m_bAltDir;
	float			m_rollSpd;

	Median m_fMed;

	AP_MOUNT		m_apMount;
};

}
#endif
