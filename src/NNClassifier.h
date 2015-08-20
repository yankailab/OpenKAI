/*
 * NNClassifier.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef SRC_NNCLASSIFIER_H_
#define SRC_NNCLASSIFIER_H_

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

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class NNClassifier
{
public:
	NNClassifier();
	~NNClassifier();

	void
	setup(const string& model_file, const string& trained_file,
			const string& mean_file, const string& label_file);

	std::vector<Prediction>
	Classify(const cv::Mat& img, int N = 5);

private:
	void
	SetMean(const string& mean_file);

	std::vector<float>
	Predict(const cv::Mat& img);

	void
	WrapInputLayer(std::vector<cv::Mat>* input_channels);

	void
	Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);

private:
	std::shared_ptr<Net<float> > net_;
	cv::Size input_geometry_;
	int num_channels_;
	cv::Mat mean_;
	std::vector<string> labels_;
};

#endif /* SRC_NNCLASSIFIER_H_ */
