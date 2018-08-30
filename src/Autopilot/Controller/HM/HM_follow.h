#ifndef OpenKAI_src_Autopilot_Controller_HM_follow_H_
#define OpenKAI_src_Autopilot_Controller_HM_follow_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"
#include "HM_base.h"

namespace kai
{

class HM_follow: public ActionBase
{
public:
	HM_follow();
	~HM_follow();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);

private:
	_ObjectBase*	m_pDet;
	HM_base* m_pHM;

	int		m_rpmSteer;
	double	m_rpmT;

	OBJECT	m_obj;
	int		m_mTargetClass;
	double	m_targetX;
	double	m_rTargetX;
	double	m_minSize;
	double	m_maxSize;

};

}


#endif

