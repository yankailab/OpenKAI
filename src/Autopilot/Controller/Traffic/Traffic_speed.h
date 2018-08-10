#ifndef OpenKAI_src_Autopilot_Controller_Traffic_speed_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_speed_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DenseFlow.h"
#include "../../ActionBase.h"
#include "Traffic_base.h"

namespace kai
{

class Traffic_speed: public ActionBase
{
public:
	Traffic_speed();
	~Traffic_speed();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	Traffic_base*	m_pTB;
	_DenseFlow*		m_pDF;
	uint64_t		m_tStampOB;

	double			m_avrSpeed;
	double			m_min;
	double			m_max;
};

}
#endif
