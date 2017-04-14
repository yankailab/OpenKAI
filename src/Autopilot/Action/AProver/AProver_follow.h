
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APROVER_FOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APROVER_FOLLOW_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Vision/Frame.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../AI/_AIbase.h"
#include "../../../Filter/FilterBase.h"
#include "../../../Navigation/_Obstacle.h"
#include "../ActionBase.h"
#include "../AProver/AProver_base.h"


namespace kai
{

class AProver_follow: public ActionBase
{
public:
	AProver_follow();
	~AProver_follow();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	void followTarget(void);

private:
	AProver_base* m_pAPM;
	_Obstacle*	m_pUniv;

	//where target should be on cam coordinate
	double	m_destX;
	double	m_destY;
	double	m_destArea;

	double	m_speedP;
	double	m_steerP;

	OBJECT* m_pTarget;
//	kai::FilterBase* m_pTargetX;
//	kai::FilterBase* m_pTargetY;
//	kai::FilterBase* m_pTargetArea;
	int		m_filterWindow;
	int		m_targetClass;



};

}

#endif

