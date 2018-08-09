#ifndef OpenKAI_src_Autopilot_Controller_Traffic_alert_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_alert_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"
#include "Traffic_base.h"

namespace kai
{

class Traffic_alert: public ActionBase
{
public:
	Traffic_alert();
	~Traffic_alert();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	Traffic_base*	m_pTB;
	uint64_t		m_tStampOB;
	double			m_avrSpeed;
};

}
#endif
