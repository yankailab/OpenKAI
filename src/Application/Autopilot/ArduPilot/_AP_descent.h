#ifndef OpenKAI_src_Autopilot_AP__AP_descent_H_
#define OpenKAI_src_Autopilot_AP__AP_descent_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Filter/FilterBase.h"
#include "../../_AutopilotBase.h"
#include "../ArduPilot/_AP_follow.h"

namespace kai
{

class _AP_descent: public _AP_follow
{
public:
	_AP_descent();
	~_AP_descent();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

protected:
	bool updateTarget(void);
	virtual bool findTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_descent *) This)->update();
		return NULL;
	}

public:
	Median m_filter;

	float m_alt;
	float m_detSize;
	vFloat4 m_vRDD;		//descents when detection inside the ROI
	float m_dTarget;	//dist to target

};
}
#endif
