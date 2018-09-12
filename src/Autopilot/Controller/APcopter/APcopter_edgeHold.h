#ifndef OpenKAI_src_Autopilot_Controller_APcopter_edgeHold_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_edgeHold_H_

#include "../../../Base/common.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Filter/Median.h"
#include "../../../Arithmetic/PolyFit.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_edgeHold: public ActionBase
{
public:
	APcopter_edgeHold();
	~APcopter_edgeHold();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	double detectEdge(void);

public:
	APcopter_base* m_pAP;
	_DepthVisionBase* m_pDV;

	PolyFit m_pf;
	Median	 m_fMed;
	vDouble3 m_vTarget;
	vDouble3 m_vPos;
	uint32_t m_iModeEnable;

	double m_edgeTop;
	double m_edgeBottom;
};

}
#endif
