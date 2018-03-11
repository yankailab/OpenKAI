#ifndef OpenKAI_src_Autopilot_Controller_HM_grass_H_
#define OpenKAI_src_Autopilot_Controller_HM_grass_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../ActionBase.h"
#include "HM_base.h"

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
	void bSetActive(bool bActive);

	_DetectorBase*	m_pDet;
	HM_base* m_pHM;

	int			m_rpmSteer;
	int			m_dir;

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

