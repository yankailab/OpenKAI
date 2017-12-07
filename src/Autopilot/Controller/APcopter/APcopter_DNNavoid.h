
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_DNNavoid_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_DNNavoid_H_

#include "../../../Base/common.h"
#include "../../../DNN/Classifier/_ImageNet.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define DNNAVOID_N_VISION 16
#define DNNAVOID_N_PLACE_CLASS 8
#define DNNAVOID_N_ACTION 8

namespace kai
{

enum DNN_AVOID_ACTION_TYPE
{
	DA_UNKNOWN,DA_SAFE,DA_WARN,DA_FORBID
};

struct DNN_AVOID_ACTION
{
	string m_pClass[DNNAVOID_N_PLACE_CLASS];
	int m_nClass;
	DNN_AVOID_ACTION_TYPE m_action;

	void init(void)
	{
		m_nClass = 0;
		m_action = DA_UNKNOWN;
	}
};

struct DNN_AVOID_VISION
{
	uint8_t m_orientation;
	OBJECT* m_pObj;
	DNN_AVOID_ACTION m_pAction[DNNAVOID_N_ACTION];
	int	m_nAction;

	void init(void)
	{
		m_orientation = 0;
		m_pObj = NULL;
		m_nAction = 0;
	}
};

class APcopter_DNNavoid: public ActionBase
{
public:
	APcopter_DNNavoid();
	~APcopter_DNNavoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

	DNN_AVOID_ACTION_TYPE str2actionType(string& strAction);
	string actionType2str(DNN_AVOID_ACTION_TYPE aType);

private:
	APcopter_base* m_pAP;
	_ImageNet*	m_pIN;

	DNN_AVOID_VISION m_pVision[DNNAVOID_N_VISION];
	int m_nVision;
	DNN_AVOID_ACTION_TYPE m_defaultAction;
	DNN_AVOID_ACTION_TYPE m_action;

};

}

#endif

