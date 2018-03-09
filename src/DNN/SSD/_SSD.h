/*
 * _SSD.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_SSD__SSD_H_
#define OpenKAI_src_DNN_SSD__SSD_H_

#include "../../Base/common.h"
#include "../../Vision/_VisionBase.h"
#include "../../Detector/_DetectorBase.h"

#ifdef USE_SSD
#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/blob.hpp>
#include <caffe/common.hpp>
#include <caffe/util/io.hpp>
#include <caffe/proto/caffe.pb.h>

namespace kai
{
using namespace caffe;
using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::shared_ptr;
using caffe::vector;

class _SSD: public _AIbase
{
public:
	_SSD();
	~_SSD();

	bool init(void* pKiss);
	void setup(const string& model_file, const string& trained_file,
			const string& mean_file, const string& label_file);
	bool link(void);
	bool start(void);
//	bool draw(void);

	std::vector<vector<float> > detect(Frame* pFrame);

private:
	void detect(void);
	void SetMean(const string& mean_file);
	void WrapInputLayer(std::vector<cv::cuda::GpuMat>* input_channels);
	void Preprocess(const cv::cuda::GpuMat& img, std::vector<cv::cuda::GpuMat>* input_channels);

	void WrapInputLayer(std::vector<cv::Mat>* input_channels);
	void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SSD*) This)->update();
		return NULL;
	}

private:
	uint64_t m_frameID;

	shared_ptr<Net<float> > net_;
	cv::Size input_geometry_;
	int num_channels_;
	cv::Mat mean_;
	Frame* m_pFrame;

	vector<string> labels_;
	double m_confidence_threshold;


};

}

#endif
#endif
