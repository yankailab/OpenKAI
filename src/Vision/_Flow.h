/*
 * _Flow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_VISION_FLOW_H_
#define SRC_VISION_FLOW_H_

#include "../Base/common.h"
#include "../Stream/_Stream.h"
#include "../Stream/FrameGroup.h"
#include "../Detector/DetectorBase.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

namespace kai
{

class _Flow:  public DetectorBase, public _ThreadBase
{
public:
	_Flow();
	virtual ~_Flow();

	bool init(Kiss* pKiss);
	bool link(void);
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
		((_Flow*) This)->update();
		return NULL;
	}

public:
	int	m_width;
	int m_height;
	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;
	_Stream*						m_pStream;
	FrameGroup*						m_pGrayFrames;
	GpuMat							m_GFlowMat;
	GpuMat							m_GDMat;

	int		m_bDepth;
	double	m_flowMax;
	double	m_flowAvr; //average flow distance in previous frame
	Frame* m_pDepth;

//	Mat m_labelColor;
//	Ptr<LookUpTable>	m_pGpuLUT;



};

} /* namespace kai */

#endif /* SRC_FLOW_H_ */
