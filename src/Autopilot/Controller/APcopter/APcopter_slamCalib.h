#ifndef OpenKAI_src_Autopilot_Controller_APcopter_slamCalib_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_slamCalib_H_

#include "../../../Base/common.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_slam.h"

namespace kai
{

class APcopter_slamCalib: public ActionBase
{
public:
	APcopter_slamCalib();
	~APcopter_slamCalib();

	virtual	bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual int check(void);
	virtual void update(void);

public:
	APcopter_base* 	m_pAP;
	APcopter_slam*	m_pSlam;

	uint32_t 		 m_apModeCalib;
	double			 m_dist;
	vector<vDouble2> m_vPos;
	double			 m_yawOffset;

};

}
#endif
