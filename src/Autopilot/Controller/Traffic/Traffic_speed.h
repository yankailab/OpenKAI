#ifndef OpenKAI_src_Autopilot_Controller_Traffic_speed_H_
#define OpenKAI_src_Autopilot_Controller_Traffic_speed_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../ActionBase.h"
#include "Traffic_base.h"

namespace kai
{

struct TRAFFIC_SPEED_BAR_TOUCH
{
	uint64_t m_trackID;
	uint64_t m_time;
};

struct TRAFFIC_SPEED_BAR
{
	vDouble4 m_roi;
	vector<TRAFFIC_SPEED_BAR_TOUCH> m_vTouch;

	void init(void)
	{
		m_roi.init();
		m_vTouch.clear();
	}

	int detect(vDouble4& tROI, uint64_t& tID)
	{
		if(tID <= 0)return -3;						//invalid tID
		if(findTouch(tID)>=0)return -2;				//trackID already recorded
		if(!bOverlapped(&m_roi, &tROI))return -1;	//not touching

		//add to touch record
		TRAFFIC_SPEED_BAR_TOUCH t;
		t.m_time = getTimeUsec();
		t.m_trackID = tID;
		m_vTouch.push_back(t);

		return m_vTouch.size()-1;
	}

	int findTouch(uint64_t& tID)
	{
		for(int i=0; i<m_vTouch.size(); i++)
		{
			IF_CONT(m_vTouch[i].m_trackID != tID);
			return i;
		}

		return -1;
	}

	uint64_t tTouch(uint64_t& tID)
	{
		int i = findTouch(tID);
		if(i<0)return 0;

		return m_vTouch[i].m_time;
	}

	void erase(uint64_t& tID)
	{
		int i = findTouch(tID);
		IF_(i<0);

		m_vTouch.erase(m_vTouch.begin()+i);
	}

	void cleanOutdated(uint64_t tDuration)
	{
		uint64_t tOut = getTimeUsec() - tDuration;

		int i;
		for(i=0; i<m_vTouch.size(); i++)
		{
			IF_CONT(m_vTouch[i].m_time < tOut);
			break;
		}

		IF_(i<=0);
		m_vTouch.erase(m_vTouch.begin(), m_vTouch.begin()+i);
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
	Traffic_base* m_pTB;
	uint64_t	 m_tStampOB;

	double m_dist;
	TRAFFIC_SPEED_BAR m_barFrom;
	TRAFFIC_SPEED_BAR m_barTo;

	double m_avrSpeed;

};

}
#endif
