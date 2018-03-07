
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_DNNnav_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_DNNnav_H_

#include "../../../Base/common.h"
#include "../../../Classifier/_ImageNet.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_DNNnav: public ActionBase
{
public:
	APcopter_DNNnav();
	~APcopter_DNNnav();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	APcopter_base* m_pAP;
#ifdef USE_TENSORRT
	_ImageNet*	m_pIN;
#endif

	int m_nTerrain;

};

}

#endif

