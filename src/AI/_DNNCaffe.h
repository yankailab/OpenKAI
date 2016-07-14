/*
 * _BVLDCaffe.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef AI__DNNCAFFE_H_
#define AI__DNNCAFFE_H_

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

class _DNNCaffe: public _ThreadBase
{
public:
	_DNNCaffe();
	~_DNNCaffe();

	void
	setup(const string& model_file, const string& trained_file,
			const string& mean_file, const string& label_file, int batch_size);

	std::vector<Prediction>
	Classify(const cv::Mat& img, int N = 5);

	std::vector<vector<Prediction> > ClassifyBatch(const vector<cv::Mat> imgs, int num_classes);

	void setModeGPU();

private:
	void SetMean(const string& mean_file);
	std::vector<float> Predict(const Mat& img);
	std::vector<float> PredictBatch(const vector<cv::Mat> imgs);
	void WrapInputLayer(vector<cv::Mat>* input_channels, int numImg);
	void WrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch);
	void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels, int iImg);
	void PreprocessBatch(const vector<cv::Mat> imgs, std::vector<std::vector<cv::Mat> >* input_batch);

	void update(void);
	static void* getUpdateThread(void* This) {
		((_DNNCaffe*) This)->update();
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

	CamFrame* m_pFrame;

};

}

#endif
