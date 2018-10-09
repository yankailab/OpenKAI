#ifndef OpenKAI_src_Autopilot_Controller_Traffic_videoOut_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_videoOut_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"
#include "Traffic_base.h"

namespace kai
{

class Traffic_videoOut: public ActionBase
{
public:
	Traffic_videoOut();
	~Traffic_videoOut();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	int check(void);

public:
	Traffic_base*	m_pTB;

	uint64_t	m_tStampOB;
	uint64_t	m_tDraw;

};

}
#endif
