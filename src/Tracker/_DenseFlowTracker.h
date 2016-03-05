/*
 * CameraOpticalFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_DENSEFLOWTRACKER_H_
#define SRC_DENSEFLOWTRACKER_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Detector/DetectorBase.h"
#include "../Base/_ThreadBase.h"
#include "stdio.h"
#include "../Camera/_CamStream.h"
#include "../Camera/FrameGroup.h"
#include "../Image/_DenseFlow.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

#define TRD_INTERVAL_DENSEFLOWTRACKER 5000

namespace kai
{

class _DenseFlowTracker:  public DetectorBase, public _ThreadBase
{
public:
	_DenseFlowTracker();
	virtual ~_DenseFlowTracker();

	bool init(JSON* pJson, string camName);
	bool start(void);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DenseFlowTracker*) This)->update();
		return NULL;
	}

public:
	_DenseFlow*	m_pDF;
	Rect	m_ROI;
	double	m_flowMax;
	double	m_flowAvr; //average flow distance in previous frame
	int		m_targetArea;

	Mat		m_Mat;
};

} /* namespace kai */

#endif /* SRC_CAMDENSEFLOW_H_ */
