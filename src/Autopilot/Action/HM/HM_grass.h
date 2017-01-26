#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMGRASS_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMGRASS_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../AI/_ImageNet.h"
#include "../ActionBase.h"
#include "HM_base.h"

#define DIR_L 0
#define DIR_F 1
#define DIR_R 2

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

	vInt2		m_grassMat;
	vDouble4	m_grassArea;
	double		m_grassMinProb;
	int			m_grassClassIdx;


};

}

#endif

