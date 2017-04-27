
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_VISUALFOLLOW_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_VISUALFOLLOW_H_

#include "../../../Base/common.h"
#include "../../../AI/_MatrixNet.h"
#include "../ActionBase.h"
#include "../APcopter/APcopter_base.h"


namespace kai
{

class APcopter_visualFollow: public ActionBase
{
public:
	APcopter_visualFollow();
	~APcopter_visualFollow();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	APcopter_base* m_pAP;
	_MatrixNet*	m_pMN;

};

}

#endif

