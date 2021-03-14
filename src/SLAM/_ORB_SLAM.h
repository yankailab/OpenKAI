/*
 * _ORB_SLAM.h
 *
 *  Created on: Jul 18, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__ORB_SLAM_H_
#define OpenKAI_src_SLAM__ORB_SLAM_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_ORB_SLAM
#include "../Vision/_VisionBase.h"
#include <System.h>
#include "Eigen/Eigen"
#include <opencv2/core/eigen.hpp>

namespace kai
{

class _ORB_SLAM: public _ModuleBase
{
public:
	_ORB_SLAM();
	virtual ~_ORB_SLAM();

	bool init(void* pKiss);
	bool start(void);
    int check(void);
	void draw(void);

	bool bTracking(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_ORB_SLAM*) This)->update();
		return NULL;
	}

public:
	_VisionBase*   m_pV;
    vInt2          m_vSize;
	ORB_SLAM3::System* m_pOS;
	uint64_t m_tStartup;

	Mat m_pose;
	vDouble3 m_vT;
	vDouble4 m_vQ;
	Mat		m_mRwc;
	Mat		m_mTwc;
	bool	m_bTracking;
	bool	m_bViewer;

};

}
#endif
#endif
#endif
