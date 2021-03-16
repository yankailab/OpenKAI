#ifndef OpenKAI_src_Autopilot_AP__AP_descent_H_
#define OpenKAI_src_Autopilot_AP__AP_descent_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Filter/FilterBase.h"
#include "../ArduPilot/_AP_follow.h"

#ifdef USE_OPENCV

namespace kai
{

class _AP_land: public _AP_follow
{
public:
	_AP_land();
	~_AP_land();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

protected:
	bool updateTarget(void);
	virtual bool findTarget(void);
	static void* getUpdate(void* This)
	{
		(( _AP_land *) This)->update();
		return NULL;
	}

public:
	Median m_filter;
	int	m_iRelayLED;

	float m_targetAlt;
    float m_zrK;
	float m_dTarget;	//dist to target

};
}
#endif
#endif
