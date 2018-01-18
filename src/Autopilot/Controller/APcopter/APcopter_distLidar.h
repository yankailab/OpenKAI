
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_distLidar_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_distLidar_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define N_LIDAR 3

namespace kai
{

struct DIST_LIDAR
{
	_DistSensorBase* m_pDS;
	uint8_t m_orientation;

	void init(void)
	{
		m_pDS = NULL;
		m_orientation = 0;
	}
};

class APcopter_distLidar: public ActionBase
{
public:
	APcopter_distLidar();
	~APcopter_distLidar();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	void updateMavlink(void);

	APcopter_base* m_pAP;
	int	m_nLidar;
	DIST_LIDAR m_pLidar[N_LIDAR];

	double m_minDH;
	double m_minDDegH;
	double m_minDV;
	double m_minDDegV;
};

}


#endif

