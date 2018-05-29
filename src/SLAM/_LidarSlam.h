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

namespace kai
{

struct LIDARSLAM_LIDAR
{
	_DistSensorBase* m_pD;

	void init(void)
	{
		m_pD = NULL;
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

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_LidarSlam*) This)->update();
		return NULL;
	}

public:
	LIDARSLAM_LIDAR* m_pDS;

};

}
#endif
