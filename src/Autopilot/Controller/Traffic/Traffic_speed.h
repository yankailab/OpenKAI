#ifndef OpenKAI_src_Autopilot_Controller_Traffic_speed_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_speed_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"
#include "Traffic_base.h"

namespace kai
{

struct TRAFFIC_SPEED_BAR
{
	vDouble4		m_roi;
	int				m_nCount;
	int				m_nCountTot;
	bool			m_bTouch;

	void init(void)
	{
		m_roi.init();
		m_nCountTot = 0;
		m_bTouch = false;
		resetCount();
	}

	void resetCount(void)
	{
		m_nCount = 0;
	}

	bool bTouch(vDouble4* pBbox)
	{
		IF_F(!bOverlapped(&m_roi, pBbox));
		return true;
	}

	void count(bool bTouch)
	{
		if(!m_bTouch && bTouch)
		{
			m_nCount++;
			m_nCountTot++;
		}

		m_bTouch = bTouch;
	}
};

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
	uint64_t		m_tStampOB;

	TRAFFIC_SPEED_BAR	m_barFrom;
	TRAFFIC_SPEED_BAR	m_barTo;
	double				m_dist;
	double				m_speed;
	uint64_t			m_tFrom;
	uint64_t			m_tTo;
	int					m_nInside;
	vDouble2			m_scaleBBox;
};

}
#endif
