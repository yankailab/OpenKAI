#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMRTH_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMRTH_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Navigation/_GPS.h"
#include "../../ActionBase.h"
#include "HM_base.h"
#include "HM_kickBack.h"

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
	HM_kickBack* m_pKB;
	_GPS* m_pGPS;

	int		m_rpmSteer;
	int		m_rpmT;
	double	m_rWP;
	double	m_dHdg;

};

}

#endif

