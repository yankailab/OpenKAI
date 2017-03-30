
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_VISUALFENCE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_VISUALFENCE_H_

#include "../../../Base/common.h"
#include "../../../AI/_ImageNet.h"
#include "../ActionBase.h"
#include "APMcopter_base.h"

namespace kai
{

class APMcopter_visualFence: public ActionBase
{
public:
	APMcopter_visualFence();
	~APMcopter_visualFence();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:

private:
	APMcopter_base* m_pAPM;
	_ImageNet*	m_pIN;

	vDouble4 m_terrainBox;
	OBJECT* m_pTerrain;


};

}

#endif

