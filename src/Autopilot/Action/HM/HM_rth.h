#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMRTH_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMRTH_H_

#include "../../../Base/common.h"
#include "../ActionBase.h"
#include "HM_base.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../AI/_MatrixNet.h"
#include "../../../Navigation/_Obstacle.h"

namespace kai
{

class HM_rth: public ActionBase
{
public:
	HM_rth();
	~HM_rth();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	HM_base* m_pHM;
	_Obstacle* m_pObs;
	_MatrixNet* m_pMN;

	double	m_steerP;
	int		m_rpmSteer;

	vDouble2 m_basePos;
	double	m_baseDist;
	int		m_iBaseClass;
	OBJECT* m_pBase;

};

}

#endif

