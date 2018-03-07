#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Detector/_ClusterNet.h"
#include "../../../Sensor/_ZEDdistance.h"
#include "../../ActionBase.h"
#include "HM_base.h"

enum AVOID_SEQUENCE
{
	av_clear,av_turn,av_markerTurn
};

namespace kai
{

class HM_avoid: public ActionBase
{
public:
	HM_avoid();
	~HM_avoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	HM_base* m_pHM;
	_ZEDdistance* m_pObs;
	_ClusterNet* m_pMN;
	int m_iMarkerClass;

	AVOID_SEQUENCE m_sequence;
	int		m_rpmSteer;
	double m_distM;
	vDouble4 m_obsBoxF;
	vDouble4 m_obsBoxL;
	vDouble4 m_obsBoxR;
	vInt2 m_posMin;
	double m_alertDist;

	uint64_t m_markerTurnStart;
	uint64_t m_markerTurnTimer;

	double m_minProb;

};

}

#endif

