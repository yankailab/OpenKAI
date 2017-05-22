#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMKICKBACK_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMKICKBACK_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Navigation/_GPS.h"
#include "../../ActionBase.h"
#include "HM_base.h"

namespace kai
{

enum KICKBACK_SEQUENCE
{
	kb_station,kb_back,kb_turn
};

class HM_kickBack: public ActionBase
{
public:
	HM_kickBack();
	~HM_kickBack();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

	void reset(void);

public:
	HM_base* m_pHM;
	_GPS* m_pGPS;

	int		m_rpmBack;
	int		m_rpmTurn;
	double	m_kickBackDist;
	double	m_targetHdg;

	KICKBACK_SEQUENCE	m_sequence;
	UTM_POS	m_wpStation;
	UTM_POS	m_wpApproach;

};

}

#endif

