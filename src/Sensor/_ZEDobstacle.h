/*
 * _ZEDobstacle.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__ZEDobstacle_H_
#define OpenKAI_src_Sensor__ZEDobstacle_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../DNN/Classifier/_ImageNet.h"
#include "../Filter/Median.h"
#include "../Vision/_ZED.h"
#include "DistSensorBase.h"

#define N_FILTER 1600

namespace kai
{

class _ZEDobstacle: public _ThreadBase, public DistSensorBase
{
public:
	_ZEDobstacle(void);
	virtual ~_ZEDobstacle();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	vInt2 matrixDim(void);

	DIST_SENSOR_TYPE type(void);
	double d(vInt4* pROI, vInt2* pPos);
	double d(vDouble4* pROI, vInt2* pPos);
	bool bReady(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ZEDobstacle *) This)->update();
		return NULL;
	}

public:
#ifdef USE_ZED
	_ZED*		m_pZed;
	bool		m_bZEDready;
#endif
	Frame*		m_pMatrix;
	vInt2		m_mDim;
	Median* 	m_pFilteredMatrix[N_FILTER];
	int			m_nFilter;

	double		m_dBlend;
};

}

#endif
