/*
 * _ORB_SLAM2.h
 *
 *  Created on: Jul 18, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__ORB_SLAM2_H_
#define OpenKAI_src_SLAM__ORB_SLAM2_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_ORB_SLAM2
#include "../Vision/_VisionBase.h"
#include <System.h>
#include <KeyFrame.h>
#include "Eigen/Eigen"
#include <opencv2/core/eigen.hpp>

namespace kai
{

class _ORB_SLAM2: public _ModuleBase
{
public:
	_ORB_SLAM2();
	virtual ~_ORB_SLAM2();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);

	bool bTracking(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_ORB_SLAM2*) This)->update();
		return NULL;
	}

public:
	int	m_width;
	int m_height;

	_VisionBase*	m_pVision;
	ORB_SLAM2::System* m_pOS;
	Frame* m_pFrame;
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
