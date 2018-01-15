
#ifndef OpenKAI_src_Autopilot_Controller_AProver_AProver_follow_H_
#define OpenKAI_src_Autopilot_Controller_AProver_AProver_follow_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Vision/Frame.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Filter/FilterBase.h"
#include "../../../Sensor/_ZEDdistance.h"
#include "../../ActionBase.h"
#include "AProver_base.h"

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
	AProver_base* m_pAP;
	_ZEDdistance*	m_pObs;

	//where target should be on cam coordinate
	double	m_destX;
	double	m_destY;
	double	m_destArea;

	double	m_speedP;
	double	m_steerP;

	OBJECT* m_pTarget;
	int		m_filterWindow;
	int		m_targetClass;

};

}

#endif

