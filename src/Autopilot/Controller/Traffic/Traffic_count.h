#ifndef OpenKAI_src_Autopilot_Controller_Traffic_count_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_count_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"
#include "Traffic_base.h"

namespace kai
{

class Traffic_count: public ActionBase
{
public:
	Traffic_count();
	~Traffic_count();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	Traffic_base*	m_pTB;
	uint64_t		m_tStampOB;

	vDouble4		m_roi;
	int				m_nCount;
	bool			m_bTouch;
	vDouble2		m_scaleBBox;

};

}
#endif
