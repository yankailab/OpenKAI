#ifndef OpenKAI_src_Autopilot_Controller_VEK_VEK_avoid_H_
#define OpenKAI_src_Autopilot_Controller_VEK_VEK_avoid_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_ZEDdistance.h"
#include "../../ActionBase.h"
#include "VEK_base.h"

namespace kai
{

class VEK_avoid: public ActionBase
{
public:
	VEK_avoid();
	~VEK_avoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	VEK_base* m_pVEK;
	_ZEDdistance* m_pObs;

	double m_vSteer;
	double m_distM;
	double m_dAlert;
	vDouble4 m_obsBox;
	vInt2 m_posMin;

};

}

#endif
