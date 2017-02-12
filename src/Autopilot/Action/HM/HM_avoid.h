#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMAVOID_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Stream/_StreamBase.h"
#include "../../../AI/_DetectNet.h"
#include "../../../Navigation/_Obstacle.h"
#include "../ActionBase.h"
#include "HM_base.h"

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
	_Obstacle* m_pObs;
	_DetectNet* m_pMarkerDN;

	vDouble4 m_obsBoxF;
	vDouble4 m_obsBoxL;
	vDouble4 m_obsBoxR;
	vInt2	m_posMin;
	double	m_alertDist;
	double	m_markerDist;

	double	m_speedP;
	double	m_steerP;

	//operation
	double m_distM;

	double		m_minProb;
	uint64_t 	m_objLifetime;
	uint64_t	m_markerTurnStart;
	uint64_t	m_markerTurnTimer;
	int			m_rpmSteer;
	OBJECT*		m_pMarker;


};

}

#endif

