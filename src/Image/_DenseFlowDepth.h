/*
 * CameraOpticalFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_DenseFlowDepth_H_
#define SRC_DenseFlowDepth_H_

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

#define TRD_INTERVAL_DENSEFLOWDEPTH 15000

namespace kai
{

class _DenseFlowDepth:  public DetectorBase, public _ThreadBase
{
public:
	_DenseFlowDepth();
	virtual ~_DenseFlowDepth();

	bool init(JSON* pJson, string name);
	bool start(void);

private:
	void findDepth(void);
	void findDepthGPU(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DenseFlowDepth*) This)->update();
		return NULL;
	}

public:
	_DenseFlow*	m_pDF;
	Rect	m_ROI;
	double	m_flowMax;
	double	m_flowAvr; //average flow distance in previous frame
	int		m_targetArea;

	CamFrame* m_pDepth;

	Mat m_labelColor;
	Ptr<LookUpTable>	m_pGpuLUT;

};

} /* namespace kai */

#endif /* SRC_CAMDENSEFLOW_H_ */
