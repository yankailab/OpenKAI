
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_DNNavoid_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_DNNavoid_H_

#include "../../../Base/common.h"
#include "../../../DNN/TensorRT/_ImageNet.h"
#include "../ActionBase.h"
#include "APcopter_base.h"

#define DNNAVOID_N_VISION 16
#define DNNAVOID_N_ACTION 8

namespace kai
{

enum DNN_AVOID_ACTION_TYPE
{
	DA_UNKNOWN,DA_SAFE,DA_WARN,DA_FORBID
};

struct DNN_AVOID_ACTION
{
	uint64_t m_mClass; //class mask
	DNN_AVOID_ACTION_TYPE m_action;

	void init(void)
	{
		m_mClass = 0;
		m_action = DA_UNKNOWN;
	}

	void addClass(int iClass)
	{
		IF_(iClass >= OBJECT_N_CLASS);
		IF_(iClass < 0);

		m_mClass |= (1 << iClass);
	}
};

struct DNN_AVOID_VISION
{
	uint8_t m_orientation;
	double 	m_angleTan;	//tangent of the degree between gravity and camera direction
	double	m_rMin;
	double	m_rMax;
	OBJECT* m_pObj;
	DNN_AVOID_ACTION m_pAction[DNNAVOID_N_ACTION];
	int	m_nAction;

	void init(void)
	{
		m_orientation = 0;
		m_angleTan = tan(30.0*DEG_RAD);
		m_pObj = NULL;
		m_nAction = 0;
		m_rMin = 0;
		m_rMax = 100;
	}
};

class APcopter_DNNavoid: public ActionBase
{
public:
	APcopter_DNNavoid();
	~APcopter_DNNavoid();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);

	DNN_AVOID_ACTION_TYPE str2actionType(string& strAction);
	string actionType2str(DNN_AVOID_ACTION_TYPE aType);

private:
#ifdef USE_TENSORRT
	_ImageNet*	m_pIN;
#endif
	APcopter_base* m_pAP;

	DNN_AVOID_VISION m_pVision[DNNAVOID_N_VISION];
	int m_nVision;
	DNN_AVOID_ACTION_TYPE m_action;

};

}

#endif

