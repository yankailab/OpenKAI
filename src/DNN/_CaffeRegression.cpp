/*
 * _CaffeRegression.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_CaffeRegression.h"

#ifdef USE_CAFFE

namespace kai
{
_CaffeRegression::_CaffeRegression()
{
	num_channels_ = 0;
	batch_size_ = 0;
}

_CaffeRegression::~_CaffeRegression()
{
}

bool _CaffeRegression::init(JSON* pJson)
{
	//Setup Caffe Classifier
	string caffeDir = "";
	string modelFile;
	string trainedFile;
	string meanFile;
	string labelFile;
	string presetDir = "";

//	CHECK_INFO(pJson->v("PRESET_DIR", &presetDir));
//	CHECK_INFO(pJson->v("CAFFE_DIR", &caffeDir));
//	CHECK_FATAL(pJson->v("CAFFE_MODEL_FILE", &modelFile));
//	CHECK_FATAL(pJson->v("CAFFE_TRAINED_FILE", &trainedFile));
//	CHECK_FATAL(pJson->v("CAFFE_MEAN_FILE", &meanFile));
//	CHECK_FATAL(pJson->v("CAFFE_LABEL_FILE", &labelFile));

	setup(caffeDir + modelFile, caffeDir + trainedFile, caffeDir + meanFile, caffeDir + labelFile, 1);
	LOG(INFO)<<"Caffe Initialized";

	double FPS = DEFAULT_FPS;
//	CHECK_ERROR(pJson->v("CAFFE_FPS", &FPS));
	this->setTargetFPS(FPS);

	m_pFrame = new Frame();

	return true;
}

void _CaffeRegression::setup(const string& model_file, const string& trained_file,
		const string& mean_file, const string& label_file, int batch_size)
{
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	batch_size_ = batch_size;

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->CopyTrainedLayersFrom(trained_file);

	CHECK_EQ(net_->num_inputs(), 1)<< "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1)<< "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file);

	/* Load labels. */
	std::ifstream labels(label_file.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file;
	string line;
	while (std::getline(labels, line))
		labels_.push_back(string(line));

	Blob<float>* output_layer = net_->output_blobs()[0];
	CHECK_EQ(labels_.size(), output_layer->channels())<< "Number of labels is different from the output layer dimension.";






//	Blob<float>* input_layer = net_->input_blobs()[0];
//	input_layer->Reshape(batch_size_, num_channels_, input_geometry_.height, input_geometry_.width);
//	/* Forward dimension change to all layers. */
//	net_->Reshape();
//	WrapBatchInputLayer(&m_input_batch);

}

void _CaffeRegression::setModeGPU(void)
{
	Caffe::set_mode(Caffe::GPU);
}

void _CaffeRegression::WrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch)
{
	Blob<float>* input_layer = net_->input_blobs()[0];

	int width = input_layer->width();
	int height = input_layer->height();
	int num = input_layer->num();
	float* input_data = input_layer->mutable_cpu_data();
	for (int j = 0; j < num; j++)
	{
		vector<cv::Mat> input_channels;
		for (int i = 0; i < input_layer->channels(); ++i)
		{
			cv::Mat channel(height, width, CV_32FC1, input_data);
			input_channels.push_back(channel);
			input_data += width * height;
		}
		input_batch->push_back(vector<cv::Mat>(input_channels));
	}

}

static bool PairCompare(const std::pair<float, int>& lhs,
		const std::pair<float, int>& rhs)
{
	return lhs.first > rhs.first;
}

/* Return the indices of the top N values of vector v. */
static std::vector<int> Argmax(const std::vector<float>& v, int N)
{
	std::vector<std::pair<float, int> > pairs;
	for (size_t i = 0; i < v.size(); ++i)
		pairs.push_back(std::make_pair(v[i], i));
	std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(),
			PairCompare);

	std::vector<int> result;
	for (int i = 0; i < N; ++i)
		result.push_back(pairs[i].second);
	return result;
}

/* Load the mean file in binaryproto format. */
void _CaffeRegression::SetMean(const string& mean_file)
{
	BlobProto blob_proto;
	ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

	/* Convert from BlobProto to Blob<float> */
	Blob<float> mean_blob;
	mean_blob.FromProto(blob_proto);

	CHECK_EQ(mean_blob.channels(), num_channels_)<< "Number of channels of mean file doesn't match input layer.";

	/* The format of the mean file is planar 32-bit float BGR or grayscale. */
	std::vector<cv::Mat> channels;
	float* data = mean_blob.mutable_cpu_data();
	for (int i = 0; i < num_channels_; ++i)
	{
		/* Extract an individual channel. */
		cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
		channels.push_back(channel);
		data += mean_blob.height() * mean_blob.width();
	}

	/* Merge the separate channels into a single image. */
	cv::Mat mean;
	cv::merge(channels, mean);

	/* Compute the global mean pixel value and create a mean image
	 * filled with this value. */
	cv::Scalar channel_mean = cv::mean(mean);
	mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
}

std::vector<vector<Prediction> > _CaffeRegression::ClassifyBatch(const vector<cv::Mat> imgs, int num_classes)
{
	vector<float> output_batch = PredictBatch(imgs);

	vector<vector<Prediction> > predictions;
	Blob<float>* output_layer = net_->output_blobs()[0];

	for (int j = 0; j < imgs.size(); j++)
	{
//		vector<float> output(output_batch.begin() + j * num_classes,
//				output_batch.begin() + (j + 1) * num_classes);
		vector<float> output(
				output_batch.begin() + j * output_layer->channels(),
				output_batch.begin() + (j + 1) * output_layer->channels());

		vector<int> maxN = Argmax(output, num_classes);

		vector<Prediction> prediction_single;

		for (int i = 0; i < num_classes; ++i)
		{
			int idx = maxN[i];
			prediction_single.push_back(make_pair(labels_[idx], output[idx]));
		}

		predictions.push_back(vector<Prediction>(prediction_single));
	}
	return predictions;
}

std::vector<float> _CaffeRegression::PredictBatch(const vector<cv::Mat> imgs)
{
	Blob<float>* input_layer = net_->input_blobs()[0];

	input_layer->Reshape(batch_size_, num_channels_, input_geometry_.height, input_geometry_.width);

	/* Forward dimension change to all layers. */
	net_->Reshape();

	std::vector<std::vector<cv::Mat> > input_batch;
	WrapBatchInputLayer(&input_batch);

	PreprocessBatch(imgs, &input_batch);

	net_->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels() * imgs.size();
	return std::vector<float>(begin, end);
}



void _CaffeRegression::PreprocessBatch(const vector<cv::Mat> imgs, std::vector<std::vector<cv::Mat> >* input_batch)
{
	for (int i = 0; i < imgs.size(); i++)
	{
		cv::Mat img = imgs[i];
		std::vector<cv::Mat> *input_channels = &(input_batch->at(i));

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

		cv::Mat sample_resized;
		if (sample.size() != input_geometry_)
			cv::resize(sample, sample_resized, input_geometry_);
		else
			sample_resized = sample;

		cv::Mat sample_float;
		if (num_channels_ == 3)
			sample_resized.convertTo(sample_float, CV_32FC3);
		else
			sample_resized.convertTo(sample_float, CV_32FC1);

		cv::Mat sample_normalized;
		cv::subtract(sample_float, mean_, sample_normalized);

		/* This operation will write the separate BGR planes directly to the
		 * input layer of the network because it is wrapped by the cv::Mat
		 * objects in input_channels. */
		cv::split(sample_normalized, *input_channels);

//        CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
//              == net_->input_blobs()[0]->cpu_data())
//          << "Input channels are not wrapping the input layer of the network.";
	}
}





void _CaffeRegression::WrapBatchInputLayerGPU(std::vector<std::vector<cv::cuda::GpuMat> > *input_batch)
{
	Blob<float>* input_layer = net_->input_blobs()[0];

	int width = input_layer->width();
	int height = input_layer->height();
	int num = input_layer->num();
	float* input_data = input_layer->mutable_gpu_data();
	for (int j = 0; j < num; j++)
	{
		vector<cv::cuda::GpuMat> input_channels;
		for (int i = 0; i < input_layer->channels(); ++i)
		{
			cv::cuda::GpuMat channel(height, width, CV_32FC1, input_data);
			input_channels.push_back(channel);
			input_data += width * height;
		}
		input_batch->push_back(vector<cv::cuda::GpuMat>(input_channels));
	}

}

std::vector<vector<Prediction> > _CaffeRegression::ClassifyBatchGPU(const vector<cv::cuda::GpuMat> imgs, int num_classes)
{
	vector<float> output_batch = PredictBatchGPU(imgs);

	vector<vector<Prediction> > predictions;
	Blob<float>* output_layer = net_->output_blobs()[0];

	for (int j = 0; j < imgs.size(); j++)
	{
//		vector<float> output(output_batch.begin() + j * num_classes,
//				output_batch.begin() + (j + 1) * num_classes);
		vector<float> output(
				output_batch.begin() + j * output_layer->channels(),
				output_batch.begin() + (j + 1) * output_layer->channels());

		vector<int> maxN = Argmax(output, num_classes);

		vector<Prediction> prediction_single;

		for (int i = 0; i < num_classes; ++i)
		{
			int idx = maxN[i];
			prediction_single.push_back(make_pair(labels_[idx], output[idx]));
		}

		predictions.push_back(vector<Prediction>(prediction_single));
	}
	return predictions;
}

std::vector<float> _CaffeRegression::PredictBatchGPU(const vector<cv::cuda::GpuMat> imgs)
{
	Blob<float>* input_layer = net_->input_blobs()[0];

	input_layer->Reshape(batch_size_, num_channels_, input_geometry_.height, input_geometry_.width);

	/* Forward dimension change to all layers. */
	net_->Reshape();

	std::vector<std::vector<cv::cuda::GpuMat> > input_batch;
	WrapBatchInputLayerGPU(&input_batch);

	PreprocessBatchGPU(imgs, &input_batch);

	net_->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels() * imgs.size();
	return std::vector<float>(begin, end);
}



void _CaffeRegression::PreprocessBatchGPU(const vector<cv::cuda::GpuMat> imgs, std::vector<std::vector<cv::cuda::GpuMat> >* input_batch)
{
	for (int i = 0; i < imgs.size(); i++)
	{
		cv::cuda::GpuMat img = imgs[i];
		std::vector<cv::cuda::GpuMat> *input_channels = &(input_batch->at(i));

		/* Convert the input image to the input image format of the network. */
		cv::cuda::GpuMat sample;
		if (img.channels() == 3 && num_channels_ == 1)
			cv::cuda::cvtColor(img, sample, CV_BGR2GRAY);
		else if (img.channels() == 4 && num_channels_ == 1)
			cv::cuda::cvtColor(img, sample, CV_BGRA2GRAY);
		else if (img.channels() == 4 && num_channels_ == 3)
			cv::cuda::cvtColor(img, sample, CV_BGRA2BGR);
		else if (img.channels() == 1 && num_channels_ == 3)
			cv::cuda::cvtColor(img, sample, CV_GRAY2BGR);
		else
			sample = img;

		cv::cuda::GpuMat sample_resized;
		if (sample.size() != input_geometry_)
			cv::cuda::resize(sample, sample_resized, input_geometry_);
		else
			sample_resized = sample;

		cv::cuda::GpuMat sample_float;
		if (num_channels_ == 3)
			sample_resized.convertTo(sample_float, CV_32FC3);
		else
			sample_resized.convertTo(sample_float, CV_32FC1);

		cv::cuda::GpuMat sample_normalized;
		cv::cuda::subtract(sample_float, mean_, sample_normalized);

		/* This operation will write the separate BGR planes directly to the
		 * input layer of the network because it is wrapped by the cv::Mat
		 * objects in input_channels. */
		cv::cuda::split(sample_normalized, *input_channels);

//        CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
//              == net_->input_blobs()[0]->cpu_data())
//          << "Input channels are not wrapping the input layer of the network.";
	}
}


}














#include <iostream>
#include <memory>
#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/layers/memory_data_layer.hpp>
#include <atltime.h>
#include <opencv2/opencv.hpp>

#define WIDTH	224
#define HEIGHT	224
#define CHANNEL	3
#define TARGET_DIM	6

using namespace caffe;
using namespace std;

vector<string> split(string str, char c){
	vector<string> v;
	string buf = "";
	stringstream ss;

	ss << str;
	while (getline(ss, buf, c)){
		v.push_back(buf);
	}

	return v;
}

void readImgListToFloat(string list_path, float *data, float *label, int data_len){

	ifstream ifs;
	string str;
	int n = 0;
	ifs.open(list_path, std::ios::in);
	if (!ifs){ LOG(INFO) << "cannot open " << list_path; return; }

	float mean[CHANNEL] = { 104, 117, 123 };

	while (getline(ifs, str)){
		vector<string> entry = split(str, '\t');
		cout << "reading: " << entry[0] << endl;
		cv::Mat img = cv::imread(entry[0]);
		cv::Mat resized_img;
		cv::resize(img, resized_img, cv::Size(WIDTH, HEIGHT));
		for (int y = 0; y < HEIGHT; y++){
			for (int x = 0; x < WIDTH; x++){
				data[y*resized_img.cols + x + resized_img.cols*resized_img.rows*0 + WIDTH * HEIGHT * CHANNEL * n]
					= resized_img.data[y*resized_img.step + x*resized_img.elemSize() + 0] - mean[0];
				data[y*resized_img.cols + x + resized_img.cols*resized_img.rows*1 + WIDTH * HEIGHT * CHANNEL * n]
					= resized_img.data[y*resized_img.step + x*resized_img.elemSize() + 1] - mean[1];
				data[y*resized_img.cols + x + resized_img.cols*resized_img.rows*2 + WIDTH * HEIGHT * CHANNEL * n]
					= resized_img.data[y*resized_img.step + x*resized_img.elemSize() + 2] - mean[2];
			}
		}
		for (int i = 0; i < TARGET_DIM; i++){
			label[n*TARGET_DIM + i] = stof(entry[i + 1]) / 256.0;
		}
		n++;
	}
}


void readImgFileName(string path, string *infiles){
	ifstream ifs;
	ifs.open(path, ios::in);
	string str;

	int n = 0;
	while (getline(ifs, str)){
		vector<string> entry = split(str, '\t');
		infiles[n] = entry[0];
		n++;
	}

	ifs.close();
}

void run_googlenet_train(){

	SolverParameter solver_param;
	ReadProtoFromTextFileOrDie("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\solver.prototxt", &solver_param);
	std::shared_ptr<Solver<float>> solver(SolverRegistry<float>::CreateSolver(solver_param));
	const auto net = solver->net();
	const auto test_net = solver->test_nets();

	net->CopyTrainedLayersFrom("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\bvlc_googlenet.caffemodel");
	test_net[0]->CopyTrainedLayersFrom("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\bvlc_googlenet.caffemodel");

	int train_data_size = 1000;
	float *train_input_data;
	float *train_label;

	int test_data_size = 1000;
	float *test_input_data;
	float *test_label;

	train_input_data	= new float[train_data_size*HEIGHT*WIDTH*CHANNEL];
	train_label			= new float[train_data_size*TARGET_DIM];
	test_input_data		= new float[test_data_size*HEIGHT*WIDTH*CHANNEL];
	test_label			= new float[test_data_size*TARGET_DIM];

	readImgListToFloat("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\train.txt", train_input_data, train_label, train_data_size);
	readImgListToFloat("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt", test_input_data, test_label, test_data_size);

	const auto train_input_layer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>( net->layer_by_name("data") );
	const auto test_input_layer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>( test_net[0]->layer_by_name("data") );
	float *train_dummy = new float[train_data_size];
	float *test_dummy = new float[test_data_size];
	for (int i = 0; i < train_data_size; i++)	train_dummy[i] = 0;
	for (int i = 0; i < test_data_size; i++)	test_dummy[i] = 0;
	train_input_layer->Reset((float*)train_input_data, (float*)train_dummy, train_data_size);
	test_input_layer->Reset((float*)test_input_data, (float*)test_dummy, test_data_size);

	const auto train_label_layer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(net->layer_by_name("label"));
	const auto test_label_layer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(test_net[0]->layer_by_name("label"));
	train_label_layer->Reset((float*)train_label, (float*)train_dummy, train_data_size);
	test_label_layer->Reset((float*)test_label, (float*)test_dummy, test_data_size);


	LOG(INFO) << "Solve start.";
	solver->Solve();

	delete[] train_input_data;	train_input_data = 0;
	delete[] train_label;		train_label = 0;
	delete[] train_dummy;		train_dummy = 0;
	delete[] test_input_data;	test_input_data = 0;
	delete[] test_label;		test_label = 0;
	delete[] test_dummy;		test_dummy = 0;
}


void run_googlenet_test(){

	int batch_size = 10;
	int test_data_size = 1000;
	int batch_iter = test_data_size / batch_size;
	int test_input_num = 2;

	string *infiles = new string[test_data_size];
	ofstream ofs;
	ofs.open("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\result.txt", ios::out);
	readImgFileName("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt", infiles);

	float *test_input_data;
	float *test_label;
	test_input_data = new float[test_data_size*HEIGHT*WIDTH*CHANNEL];
	test_label = new float[test_data_size*TARGET_DIM];

	Net<float> test_net("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\deploy.prototxt", TEST);
	test_net.CopyTrainedLayersFrom("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\snapshot\\bvlc_googlenet_iter_80000.caffemodel");

	readImgListToFloat("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt", test_input_data, test_label, test_data_size);

	CFileTime cTimeStart, cTimeEnd;
	CFileTimeSpan cTimeSpan;
	cTimeStart = CFileTime::GetCurrentTime();

	const auto input_test_layer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(test_net.layer_by_name("data"));
	float *test_dummy = new float[test_data_size];
	for (int i = 0; i < test_data_size; i++)	test_dummy[i] = 0.0f;
	cv::Mat oimg;
	cv::Mat resized_oimg;

	for (int batch = 0; batch < batch_iter; batch++){
		input_test_layer->Reset((float*)test_input_data + batch * WIDTH*HEIGHT*CHANNEL * batch_size, test_dummy + batch * batch_size, batch_size);
		const auto result = test_net.Forward();

		const auto data = result[1]->cpu_data();
		for (int i = 0; i < batch_size; i++){
			int total_id = batch * batch_size + i;
			oimg = cv::imread(infiles[total_id]);
			resized_oimg;
			cv::resize(oimg, resized_oimg, cv::Size(WIDTH, HEIGHT));

			int *val = new int(TARGET_DIM);
			for (int j = 0; j < TARGET_DIM; j++){
				val[j] = (int)(data[i*TARGET_DIM + j] * 256);
				if (val[j] < 0)	val[j] = 0;
				if (j == 0 && val[j] > WIDTH)	val[j] = WIDTH - 1;
				if (j == 1 && val[j] > HEIGHT)	val[j] = HEIGHT - 1;
				if (j >= 3 && val[j] > 255)		val[j] = 255;
			}
			cv::circle(resized_oimg, cv::Point(val[0], val[1]), val[2], cv::Scalar(val[3], val[4], val[5]), 3);

			stringstream ofname;
			ofname << "G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\output_img\\" << setw(5) << setfill('0') << to_string(total_id) << ".png";
			cv::imwrite(ofname.str(), resized_oimg);
		}
	}
	cTimeEnd = CFileTime::GetCurrentTime();
	cTimeSpan = cTimeEnd - cTimeStart;
	cerr << "testing time : " << cTimeSpan.GetTimeSpan() / 10000 << "[ms]" << endl;

	ofs.close();
	oimg.release();
	resized_oimg.release();
	delete[] test_input_data;	test_input_data = 0;
	delete[] test_label;		test_label = 0;
	delete[] test_dummy;		test_dummy = 0;
	delete[] infiles;			infiles = 0;
}


int main(int argc, char **argv){
	FLAGS_alsologtostderr = 1;
	GlobalInit(&argc, &argv);

	Caffe::set_mode(Caffe::GPU);

	//run_googlenet_train();
	run_googlenet_test();

	return 0;
}


#endif
