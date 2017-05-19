/*
 * _CaffeRegression.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__CaffeRegression_H_
#define OPENKAI_SRC_DNN__CaffeRegression_H_

#include "../Base/common.h"

#ifdef USE_CAFFE

#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/blob.hpp>
#include <caffe/common.hpp>
#include <caffe/util/io.hpp>
#include <caffe/proto/caffe.pb.h>

#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../Base/cv.h"
#include "../Base/_ThreadBase.h"
#include "../Stream/Frame.h"


namespace kai
{

using namespace caffe;
// NOLINT(build/namespaces)
using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::shared_ptr;
using caffe::vector;
using std::string;
using namespace std;
using namespace cv;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class _CaffeRegression: public _ThreadBase
{
public:
	_CaffeRegression();
	~_CaffeRegression();

	bool init(JSON* pJson);
	void setup(const string& model_file, const string& trained_file,
			const string& mean_file, const string& label_file, int batch_size);

	std::vector<Prediction> Classify(const cv::Mat& img, int N = 5);
	std::vector<vector<Prediction> > ClassifyBatch(const vector<cv::Mat> imgs, int num_classes);
	std::vector<vector<Prediction> > ClassifyBatchGPU(const vector<cv::cuda::GpuMat> imgs, int num_classes);

	void setModeGPU();

private:
	void SetMean(const string& mean_file);
	std::vector<float> Predict(const Mat& img);
	std::vector<float> PredictBatch(const vector<cv::Mat> imgs);
	std::vector<float> PredictBatchGPU(const vector<cv::cuda::GpuMat> imgs);
	void WrapInputLayer(vector<cv::Mat>* input_channels, int numImg);
	void WrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch);
	void WrapBatchInputLayerGPU(std::vector<std::vector<cv::cuda::GpuMat> > *input_batch);
	void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels, int iImg);
	void PreprocessBatch(const vector<cv::Mat> imgs, std::vector<std::vector<cv::Mat> >* input_batch);
	void PreprocessBatchGPU(const vector<cv::cuda::GpuMat> imgs, std::vector<std::vector<cv::cuda::GpuMat> >* input_batch);

	void update(void);
	static void* getUpdateThread(void* This) {
		((_CaffeRegression*) This)->update();
		return NULL;
	}


private:
	shared_ptr<Net<float> > net_;
	cv::Size input_geometry_;
	int num_channels_;
	cv::Mat mean_;
	vector<string> labels_;
	int batch_size_;
//	std::vector<std::vector<cv::Mat> > m_input_batch;

	Frame* m_pFrame;

};

}

#endif

#endif
