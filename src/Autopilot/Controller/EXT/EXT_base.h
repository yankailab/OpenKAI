#ifndef OpenKAI_src_Autopilot_Controller_EXT_EXT_base_H_
#define OpenKAI_src_Autopilot_Controller_EXT_EXT_base_H_

#include "../../../Base/common.h"
#include "../../../IO/_IOBase.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../ActionBase.h"

namespace kai
{

class EXT_base: public ActionBase
{
public:
	EXT_base();
	~EXT_base();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	_IOBase* m_pIO;
	_DetectorBase* m_pDB;
	int	m_iTargetClass;

};

}

#endif

