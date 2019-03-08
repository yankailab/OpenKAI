#ifndef OpenKAI_src_Autopilot_Controller_Traffic_base_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_base_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../ActionBase.h"

namespace kai
{

class Traffic_base: public ActionBase
{
public:
	Traffic_base();
	~Traffic_base();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);

	bool bInsideROI(vDouble4& bb);

public:
	_DetectorBase* m_pOB;

	vector<Point2f>	m_vROI;


};

}
#endif
