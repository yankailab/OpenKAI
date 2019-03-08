#ifndef OpenKAI_src_Autopilot_Controller_Traffic_speed_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_speed_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Filter/Median.h"
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
	void update(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

public:
	Traffic_base*	m_pTB;
	uint64_t		m_tStampOB;
	OBJECT_DARRAY	m_obj;
	OBJECT_DARRAY	m_objHdgAlert;
	OBJECT_DARRAY	m_objSpeedAlert;

	vDouble2		m_vHdgLimit;
	double			m_hdgMeasureMinSpeed;
	vDouble2		m_vSpeedLimit;
	double			m_kSpeed;
	double			m_kSpeedX;
	double			m_kSpeedY;
	double			m_avrSpeed;
	double			m_drawVscale;

	int				m_nAlert;
	int				m_nMed;
	Median			m_fNspeedAlert;

};

}
#endif
