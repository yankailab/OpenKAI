/*
 * _LidarSlam.h
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__LidarSlam_H_
#define OpenKAI_src_SLAM__LidarSlam_H_

#include "../Base/common.h"
#include "../Sensor/_DistSensorBase.h"
#include "_SlamBase.h"

#define LIDARSLAM_N_LIDAR 3

namespace kai
{

struct LIDARSLAM_LIDAR
{
	_DistSensorBase* m_pD;
	double	m_d;

	void init(void)
	{
		m_pD = NULL;
		m_d = -1;
	}

	void update(void)
	{
		IF_(!m_pD);
		m_d = m_pD->dMin();
	}

	bool bLock(void)
	{
		IF_F(m_d < 0.0);
		return true;
	}
};

class _LidarSlam: public _SlamBase
{
public:
	_LidarSlam();
	virtual ~_LidarSlam();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	bool cli(int& iY);

	void reset(void);

private:
	void updateAxis(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_LidarSlam*) This)->update();
		return NULL;
	}

public:
	LIDARSLAM_LIDAR m_pDS[LIDARSLAM_N_LIDAR];
	int	m_nDS;

};

}
#endif
