/*
 * SegNet.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "SegNet.h"
namespace kai
{

SegNet::SegNet()
{
	num_channels_ = 0;
}

SegNet::~SegNet()
{
}

void SegNet::setup(const string& model_file, const string& trained_file,
		const string& color_file)
{
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->CopyTrainedLayersFrom(trained_file);

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	input_size_ = cv::Size(input_layer->width(), input_layer->height());

	Blob<float>* output_layer = net_->output_blobs()[0];

	m_labelColor = imread(color_file, 1);//CV_LOAD_IMAGE_ANYDEPTH);//IMREAD_COLOR);
}

cv::Mat SegNet::segment(cv::Mat img)
{
	int i;

	Blob<float>* input_layer = net_->input_blobs()[0];
	input_layer->Reshape(1, num_channels_, input_size_.height, input_size_.width);

	net_->Reshape();

	std::vector<cv::Mat> input_channels;
	WrapInputLayer(&input_channels);
	Preprocess(img, &input_channels);

	net_->ForwardPrefilled();

	Blob<float>* output_layer = net_->output_blobs()[0];//net_->blob_by_name("argmax").get();
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels() * input_size_.area() * 4;
	vector<float> pOutput = std::vector<float>(begin, end);

	Mat segIdx = Mat(input_size_.height, input_size_.width, CV_32FC1, pOutput.data());

	Mat uimg,uimg3;
	segIdx.convertTo(uimg, CV_8UC1);
	cvtColor(uimg, uimg3, CV_GRAY2BGR);

	Mat segRGB = Mat(input_size_.height, input_size_.width, CV_8UC3);
	cv::LUT(uimg3,m_labelColor,segRGB);

	return segRGB;

}

/* Wrap the input layer of the network in separate cv::Mat objects
 * (one per channel). This way we save one memcpy operation and we
 * don't need to rely on cudaMemcpy2D. The last preprocessing
 * operation will write the separate channels directly to the input
 * layer. */
void SegNet::WrapInputLayer(std::vector<cv::Mat>* input_channels)
{
	Blob<float>* input_layer = net_->input_blobs()[0];

	float* input_data = input_layer->mutable_cpu_data();
	for (int i = 0; i < input_layer->channels(); ++i)
	{
		cv::Mat channel(input_size_.height, input_size_.width, CV_32FC1, input_data);
		input_channels->push_back(channel);
		input_data += input_size_.width * input_size_.height;
	}
}

void SegNet::Preprocess(const cv::Mat& img,
		std::vector<cv::Mat>* input_channels)
{
	/* Convert the input image to the input image format of the network. */
	cv::Mat sample;
	if (img.channels() == 3 && num_channels_ == 1)
		cv::cvtColor(img, sample, CV_BGR2GRAY);
	else if (img.channels() == 4 && num_channels_ == 1)
		cv::cvtColor(img, sample, CV_BGRA2GRAY);
	else if (img.channels() == 4 && num_channels_ == 3)
		cv::cvtColor(img, sample, CV_BGRA2BGR);
	else if (img.channels() == 1 && num_channels_ == 3)
		cv::cvtColor(img, sample, CV_GRAY2BGR);
	else
		sample = img;
//		cv::cvtColor(img, sample, CV_BGR2RGB);

	cv::Mat sample_resized;
	if (sample.size() != input_size_)
		cv::resize(sample, sample_resized, input_size_);
	else
		sample_resized = sample;

	cv::Mat sample_float;
	if (num_channels_ == 3)
		sample_resized.convertTo(sample_float, CV_32FC3);
	else
		sample_resized.convertTo(sample_float, CV_32FC1);

	/* This operation will write the separate BGR planes directly to the
	 * input layer of the network because it is wrapped by the cv::Mat
	 * objects in input_channels. */
	cv::split(sample_float, *input_channels);

//	CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
//			== net_->input_blobs()[0]->cpu_data())
//															<< "Input channels are not wrapping the input layer of the network.";
}



}
