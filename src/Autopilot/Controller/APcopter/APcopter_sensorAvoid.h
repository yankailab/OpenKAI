
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_sensorAVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_sensorAVOID_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_ZEDobstacle.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define N_DIST_SENSOR 32
#define N_DS_SEGMENT 16

namespace kai
{

struct DIST_SEGMENT
{
	uint8_t			m_orient;
	vInt4			m_roi;
	int				m_iSeg;

	void init(void)
	{
		m_orient = 0;
		m_roi.init();
		m_iSeg = 0;
	}
};

struct DIST_SENSOR
{
	DistSensorBase*	m_pSensor;
	int				m_nSeg;
	DIST_SEGMENT	m_pSeg[N_DS_SEGMENT];

	void init(void)
	{
		m_pSensor = NULL;
		m_nSeg = 0;
	}
};

class APcopter_sensorAvoid: public ActionBase
{
public:
	APcopter_sensorAvoid();
	~APcopter_sensorAvoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	void updateMavlink(void);

	APcopter_base* m_pAP;
	int	m_nDS;
	DIST_SENSOR m_pDS[N_DIST_SENSOR];

};

}

#endif

