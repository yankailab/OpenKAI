/*
 * _LSD_SLAM.h
 *
 *  Created on: Jul 26, 2017
 *      Author: yankai
 */

#ifndef SRC_SLAM_LSD_SLAM_H_
#define SRC_SLAM_LSD_SLAM_H_

#include "../Base/common.h"

#ifdef USE_LSD_SLAM

#include "../Vision/_VisionBase.h"
#include "Eigen/Eigen"
#include <opencv2/core/eigen.hpp>

#include "LSDSLAM.h"

namespace kai
{

class _LSD_SLAM: public _ThreadBase
{
public:
	_LSD_SLAM();
	virtual ~_LSD_SLAM();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	bool bTracking(void);
	void reset(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_LSD_SLAM*) This)->update();
		return NULL;
	}

public:
	int	m_width;
	int m_height;
	bool m_bViewer;

	_VisionBase*	m_pVision;
	LSDSLAM*	m_pLSD;
	Frame* m_pFrame;

};

}

#endif
#endif
