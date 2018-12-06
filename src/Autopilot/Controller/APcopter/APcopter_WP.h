#ifndef OpenKAI_src_Autopilot_Controller_APcopter_WP_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_WP_H_

#include "../../../Base/common.h"
#include "../../../Mission/Waypoint.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_WP: public ActionBase
{
public:
	APcopter_WP();
	~APcopter_WP();

	virtual	bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual int check(void);
	virtual void update(void);

public:
	APcopter_base* 	m_pAP;
};

}
#endif
