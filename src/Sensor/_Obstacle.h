/*
 * _Obstacle.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION_OBSTACLE_H_
#define SRC_NAVIGATION_OBSTACLE_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../DNN/Detector/_ImageNet.h"
#include "../Filter/Median.h"
#include "../Sensor/DistSensorBase.h"
#include "../Vision/_ZED.h"

#define N_FILTER 1600

namespace kai
{

class _Obstacle: public _ThreadBase, public DistSensorBase
{
public:
	_Obstacle(void);
	virtual ~_Obstacle();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	vInt2 matrixDim(void);

	DIST_SENSOR_TYPE type(void);
	double d(vInt4* pROI, vInt2* pPos);
	double d(vDouble4* pROI, vInt2* pPos);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Obstacle *) This)->update();
		return NULL;
	}

public:
	_ZED*		m_pZed;
	Frame*		m_pMatrix;
	vInt2		m_mDim;
	Median* 	m_pFilteredMatrix[N_FILTER];
	int			m_nFilter;

	double		m_dBlend;
};

}

#endif
