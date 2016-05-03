/*
 * CameraOpticalFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_ROITRACKER_H_
#define SRC_ROITRACKER_H_

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

namespace kai
{

class _ROITracker:  public DetectorBase, public _ThreadBase
{
public:
	_ROITracker();
	virtual ~_ROITracker();

	bool init(JSON* pJson, string camName);
	bool start(void);

	void setROI(Rect2d roi);
	void tracking(bool bTracking);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ROITracker*) This)->update();
		return NULL;
	}

public:
	_CamStream*	m_pCamStream;
	Rect2d	m_ROI;
	Rect2d	m_newROI;
	Mat*	m_pMat;
	bool	m_bTracking;

	Ptr<Tracker> m_pTracker;
};

} /* namespace kai */

#endif /* SRC_CAMDENSEFLOW_H_ */
