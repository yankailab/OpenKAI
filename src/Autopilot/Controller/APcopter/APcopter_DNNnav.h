
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APcopter_DNNnav_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APcopter_DNNnav_H_

#include "../../../Base/common.h"
#include "../../../DNN/Classifier/_ImageNet.h"
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
	_ImageNet*	m_pIN;

	int m_nTerrain;

};

}

#endif

