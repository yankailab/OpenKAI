#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMRTHAPPROACH_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMRTHAPPROACH_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../DNN/Detector/_ClusterNet.h"
#include "../../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_rth_approach: public ActionBase
{
public:
	HM_rth_approach();
	~HM_rth_approach();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	HM_base* m_pHM;
	_ClusterNet* m_pMN;

	int		m_rpmT;
	int		m_rpmSteer;
	double	m_targetX;
	double	m_rTargetX;
	OBJECT* m_pTarget;
	int		m_iTargetClass;
	string	m_targetName;


};

}

#endif

