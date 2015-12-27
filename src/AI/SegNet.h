/*
 * SegNet.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef AI_SegNet_H_
#define AI_SegNet_H_

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

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class SegNet
{
public:
	SegNet();
	~SegNet();

	void setup(const string& model_file, const string& trained_file,
			const string& color_file);
	cv::Mat segment(cv::Mat img);

private:
//	std::vector<float> Predict(const cv::Mat& img);
	void WrapInputLayer(std::vector<cv::Mat>* input_channels);
	void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);

private:
	shared_ptr<Net<float> > net_;
	cv::Size input_size_;
	int num_channels_;
	cv::Mat m_labelColor;

	vector<string> labels_;
};

}

#endif /* SRC_SegNet_H_ */
