/*
 * CameraOpticalFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_DENSEFLOW_H_
#define SRC_DENSEFLOW_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Detector/DetectorBase.h"
#include "../Base/_ThreadBase.h"
#include "stdio.h"
#include "../Camera/_CamStream.h"
#include "../Camera/FrameGroup.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

namespace kai
{

class _DenseFlow:  public DetectorBase, public _ThreadBase
{
public:
	_DenseFlow();
	virtual ~_DenseFlow();

	bool init(JSON* pJson, string camName);
	bool start(void);

private:
	inline bool isFlowCorrect(Point2f u);
	Vec3b computeColor(float fx, float fy);
	void drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion);
	void generateFlowMap(const GpuMat& d_flow);

	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DenseFlow*) This)->update();
		return NULL;
	}

public:
	fVector4	m_flow;
	GpuMat		m_GFlowMat;
//	Mat 		m_cMat;

	int	m_width;
	int m_height;

	Mat m_cFlowMat;
	Mat m_flowX;
	Mat m_flowY;

	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;

	_CamStream*			m_pCamStream;
	FrameGroup*			m_pGrayFrames;

	bool		m_bDepth;

	double	m_flowMax;
	double	m_flowAvr; //average flow distance in previous frame
	int		m_targetArea;

	CamFrame* m_pDepth;
	CamFrame* m_pSeg;

	Mat m_labelColor;
	Ptr<LookUpTable>	m_pGpuLUT;


};

} /* namespace kai */

#endif /* SRC_CAMDENSEFLOW_H_ */
