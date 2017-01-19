#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMGRASS_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMGRASS_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../AI/_ImageNet.h"
#include "../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_grass: public ActionBase
{
public:
	HM_grass();
	~HM_grass();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	_ImageNet*	m_pIN;
	HM_base* m_pHM;

	double	m_speedP;
	double	m_steerP;


};

}

#endif

