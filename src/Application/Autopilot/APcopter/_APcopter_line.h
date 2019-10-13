#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_line_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_line_H_

#include "../../../Base/common.h"
#include "../../../Detector/_Line.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_base.h"
#include "_APcopter_posCtrl.h"

namespace kai
{

class _APcopter_line: public _AutopilotBase
{
public:
	_APcopter_line();
	~_APcopter_line();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);
	virtual bool find(void);
	virtual bool bFound(void);

public:
	_APcopter_base*		m_pAP;
	_APcopter_posCtrl*	m_pPC;
	_Line*				m_pDet;
	_DistSensorBase*	m_pDS;

	OBJECT			m_tO;
	bool			m_bFound;

	//roll, pitch, alt, yaw
	vDouble4 		m_vTargetP;
	vDouble4 		m_vSetP;
	bool			m_bAltDir;

	Median m_fMed;

	AP_MOUNT		m_apMount;
};

}
#endif
