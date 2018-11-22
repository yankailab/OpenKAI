#ifndef OpenKAI_src_Autopilot_Controller_APcopter_slamCalib_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_slamCalib_H_

#include "../../../Base/common.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_slamCalib: public ActionBase
{
public:
	APcopter_slamCalib();
	~APcopter_slamCalib();

	virtual	bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool cli(int& iY);
	virtual int check(void);
	virtual void update(void);

public:
	APcopter_base* 	m_pAP;

	vector<vDouble2> m_vPos;
	bool			 m_bCalibrating;
	uint32_t 		 m_apModeCalib;
	float			 m_yawOffset;

};

}
#endif
