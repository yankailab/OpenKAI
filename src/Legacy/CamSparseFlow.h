/*
 * CameraFeatureFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMSPARSEFLOW_H_
#define SRC_CAMSPARSEFLOW_H_

#include "../Base/common.h"
#include "stdio.h"
#include "../Base/cvplatform.h"
#include "../Stream/_Stream.h"
#include "../Stream/FrameGroup.h"

namespace kai
{

class CamSparseFlow
{
public:
	CamSparseFlow();
	virtual ~CamSparseFlow();

	bool init(void);
	fVec4 detect(_Stream* pFrame);

private:
	fVec4 m_flow;
	fVec3 m_diffPrev;

#ifdef USE_CUDA
	Ptr<cuda::CornersDetector> m_pDetector;
	Ptr<cuda::SparsePyrLKOpticalFlow> m_pPyrLK;

	unsigned int m_numCorners;

#else
	std::vector<cv::KeyPoint> m_keypoint1;
	std::vector<cv::KeyPoint> m_keypoint2;

	cv::Ptr<cv::DescriptorExtractor> m_pExtractor;
	cv::UMat m_descriptor1;
	cv::UMat m_descriptor2;

	cv::Ptr<cv::DescriptorMatcher> m_pMatcher;
	std::vector<cv::DMatch> m_dMatch;

#endif
};

} /* namespace kai */

#endif /* SRC_CAMSPARSEFLOW_H_ */
