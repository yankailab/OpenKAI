#ifndef OpenKAI_src_Autopilot_Controller_Traffic_base_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_base_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"

namespace kai
{

class Traffic_base: public ActionBase
{
public:
	Traffic_base();
	~Traffic_base();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);

public:
	_ObjectBase* m_pOB;

};

}
#endif
