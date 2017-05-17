#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_HMGRASS_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_HMGRASS_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../DNN/_ImageNet.h"
#include "../ActionBase.h"
#include "HM_base.h"

namespace kai
{

enum GRASSTURN_SEQUENCE
{
	gt_grass,gt_timerSet,gt_turn,gt_randomTurn
};

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
	void bSetActive(bool bActive);

	_ImageNet*	m_pIN;
	HM_base* m_pHM;

	int			m_rpmSteer;
	uint64_t	m_turnTimer;
	uint64_t	m_tTurnSet;

	GRASSTURN_SEQUENCE m_sequence;

	uint64_t	m_nTurnRand;
	uint64_t	m_tTurnRandRange;
	uint64_t	m_tTurnRand;

	double		m_grassMinProb;
	vDouble4	m_grassBoxL;
	vDouble4	m_grassBoxF;
	vDouble4	m_grassBoxR;

	int 		m_iGrassClass;
	OBJECT*		m_pGrassL;
	OBJECT*		m_pGrassF;
	OBJECT*		m_pGrassR;

};

}

#endif

