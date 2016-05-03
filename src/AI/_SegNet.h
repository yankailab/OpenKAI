/*
 * SegNet.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef AI_SEGNET_H_
#define AI_SEGNET_H_

#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/blob.hpp>
#include <caffe/common.hpp>
#include <caffe/util/io.hpp>
#include <caffe/proto/caffe.pb.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Base/_ThreadBase.h"
#include "../Camera/CamFrame.h"
#include "../Camera/_CamStream.h"

namespace kai
{

using namespace caffe;
// NOLINT(build/namespaces)
using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::shared_ptr;
using caffe::vector;
using caffe::MemoryDataLayer;
using std::string;
using namespace std;
using namespace cv;

class _SegNet: public _ThreadBase
{
public:
	_SegNet();
	~_SegNet();

	bool init(string name, JSON* pJson);
	bool start(void);

private:
	void segment(void);
	void WrapInputLayer(std::vector<cv::Mat>* input_channels);
	void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);

	void segmentGPU(void);
	void WrapInputLayerGPU(std::vector<cv::cuda::GpuMat>* input_channels);
	void PreprocessGPU(std::vector<cv::cuda::GpuMat>* input_channels);

	void update(void);
	static void* getUpdateThread(void* This) {
		((_SegNet*) This)->update();
		return NULL;
	}

public:
	_CamStream* m_pCamStream;
	int			m_cudaDeviceID;
	Mat m_segment;

private:
	shared_ptr<Net<float> > net_;
	Size m_InputSize;
	int m_NumChannels;
	Mat m_labelColor;
	Ptr<LookUpTable>	m_pGpuLUT;

	CamFrame* m_pFrame;
	CamFrame* m_pSegment;
	Mat m_frame;

};

}

#endif /* SRC_SegNet_H_ */
