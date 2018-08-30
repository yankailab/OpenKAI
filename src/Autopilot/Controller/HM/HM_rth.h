#ifndef OpenKAI_src_Autopilot_Controller_HM_rth_H_
#define OpenKAI_src_Autopilot_Controller_HM_rth_H_

#include "../../../Base/common.h"
#include "../../../Automaton/_Automaton.h"
#include "../../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_rth: public ActionBase
{
public:
	HM_rth();
	~HM_rth();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);

private:
	HM_base* m_pHM;

	int		m_rpmSteer;
	int		m_rpmT;

};

}
#endif
