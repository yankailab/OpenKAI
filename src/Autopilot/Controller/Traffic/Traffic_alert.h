#ifndef OpenKAI_src_Autopilot_Controller_Traffic_alert_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_alert_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"
#include "Traffic_base.h"

namespace kai
{

struct TRAFFIC_ALERT
{
	uint64_t m_mClass;

	void init(void)
	{
		m_mClass = 0;
	}

	bool addClass(int iClass)
	{
		IF_F(iClass < 0);

		m_mClass |= (1<<iClass);

		return true;
	}

	bool bAlert(int iClass)
	{
		IF_F(iClass < 0);
		return m_mClass & (1<<iClass);
	}

	bool bAlert(uint64_t mClass)
	{
		return m_mClass & mClass;
	}
};

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

	OBJECT_DARRAY	m_obj;
	TRAFFIC_ALERT	m_alert;
};

}
#endif
