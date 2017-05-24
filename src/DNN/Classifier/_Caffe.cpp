/*
 * _Caffe.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_Caffe.h"

#ifdef USE_CAFFE

namespace kai
{
_Caffe::_Caffe()
{
	m_nChannel = 0;
	m_batchSize = 0;
	m_pFrame = NULL;
}

_Caffe::~_Caffe()
{
}

bool _Caffe::init(void* pKiss)
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

void _Caffe::setup(const string& model_file, const string& trained_file,
		const string& mean_file, const string& label_file, int batch_size)
{
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	m_batchSize = batch_size;

	/* Load the network. */
	m_pNet.reset(new Net<float>(model_file, TEST));
	m_pNet->CopyTrainedLayersFrom(trained_file);

	CHECK_EQ(m_pNet->num_inputs(), 1)<< "Network should have exactly one input.";
	CHECK_EQ(m_pNet->num_outputs(), 1)<< "Network should have exactly one output.";

	Blob<float>* input_layer = m_pNet->input_blobs()[0];
	m_nChannel = input_layer->channels();
	CHECK(m_nChannel == 3 || m_nChannel == 1)<< "Input layer should have 1 or 3 channels.";
	m_inputGeometry = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file);

	/* Load labels. */
	std::ifstream labels(label_file.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file;
	string line;
	while (std::getline(labels, line))
		m_vLabel.push_back(string(line));

	Blob<float>* output_layer = m_pNet->output_blobs()[0];
	CHECK_EQ(m_vLabel.size(), output_layer->channels())<< "Number of labels is different from the output layer dimension.";






//	Blob<float>* input_layer = net_->input_blobs()[0];
//	input_layer->Reshape(batch_size_, num_channels_, input_geometry_.height, input_geometry_.width);
//	/* Forward dimension change to all layers. */
//	net_->Reshape();
//	WrapBatchInputLayer(&m_input_batch);

}

void _Caffe::setModeGPU(void)
{
	Caffe::set_mode(Caffe::GPU);
}

void _Caffe::WrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch)
{
	Blob<float>* input_layer = m_pNet->input_blobs()[0];

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
void _Caffe::SetMean(const string& mean_file)
{
//	BlobProto blob_proto;
//	ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);
//
//	/* Convert from BlobProto to Blob<float> */
//	Blob<float> mean_blob;
//	mean_blob.FromProto(blob_proto);
//
//	CHECK_EQ(mean_blob.channels(), num_channels_)<< "Number of channels of mean file doesn't match input layer.";
//
//	/* The format of the mean file is planar 32-bit float BGR or grayscale. */
//	std::vector<cv::Mat> channels;
//	float* data = mean_blob.mutable_cpu_data();
//	for (int i = 0; i < num_channels_; ++i)
//	{
//		/* Extract an individual channel. */
//		cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
//		channels.push_back(channel);
//		data += mean_blob.height() * mean_blob.width();
//	}
//
//	/* Merge the separate channels into a single image. */
//	cv::Mat mean;
//	cv::merge(channels, mean);
//
//	/* Compute the global mean pixel value and create a mean image
//	 * filled with this value. */
//	cv::Scalar channel_mean = cv::mean(mean);
//	mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
}

std::vector<vector<Prediction> > _Caffe::ClassifyBatch(const vector<cv::Mat> imgs, int num_classes)
{
	vector<float> output_batch = PredictBatch(imgs);

	vector<vector<Prediction> > predictions;
	Blob<float>* output_layer = m_pNet->output_blobs()[0];

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
			prediction_single.push_back(make_pair(m_vLabel[idx], output[idx]));
		}

		predictions.push_back(vector<Prediction>(prediction_single));
	}
	return predictions;
}

std::vector<float> _Caffe::PredictBatch(const vector<cv::Mat> imgs)
{
	Blob<float>* input_layer = m_pNet->input_blobs()[0];

	input_layer->Reshape(m_batchSize, m_nChannel, m_inputGeometry.height, m_inputGeometry.width);

	/* Forward dimension change to all layers. */
	m_pNet->Reshape();

	std::vector<std::vector<cv::Mat> > input_batch;
	WrapBatchInputLayer(&input_batch);

	PreprocessBatch(imgs, &input_batch);

	m_pNet->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = m_pNet->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels() * imgs.size();
	return std::vector<float>(begin, end);
}



void _Caffe::PreprocessBatch(const vector<cv::Mat> imgs, std::vector<std::vector<cv::Mat> >* input_batch)
{
	for (int i = 0; i < imgs.size(); i++)
	{
		cv::Mat img = imgs[i];
		std::vector<cv::Mat> *input_channels = &(input_batch->at(i));

		/* Convert the input image to the input image format of the network. */
		cv::Mat sample;
		if (img.channels() == 3 && m_nChannel == 1)
			cv::cvtColor(img, sample, CV_BGR2GRAY);
		else if (img.channels() == 4 && m_nChannel == 1)
			cv::cvtColor(img, sample, CV_BGRA2GRAY);
		else if (img.channels() == 4 && m_nChannel == 3)
			cv::cvtColor(img, sample, CV_BGRA2BGR);
		else if (img.channels() == 1 && m_nChannel == 3)
			cv::cvtColor(img, sample, CV_GRAY2BGR);
		else
			sample = img;

		cv::Mat sample_resized;
		if (sample.size() != m_inputGeometry)
			cv::resize(sample, sample_resized, m_inputGeometry);
		else
			sample_resized = sample;

		cv::Mat sample_float;
		if (m_nChannel == 3)
			sample_resized.convertTo(sample_float, CV_32FC3);
		else
			sample_resized.convertTo(sample_float, CV_32FC1);

		cv::Mat sample_normalized;
		cv::subtract(sample_float, m_mMean, sample_normalized);

		/* This operation will write the separate BGR planes directly to the
		 * input layer of the network because it is wrapped by the cv::Mat
		 * objects in input_channels. */
		cv::split(sample_normalized, *input_channels);

//        CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
//              == net_->input_blobs()[0]->cpu_data())
//          << "Input channels are not wrapping the input layer of the network.";
	}
}





void _Caffe::WrapBatchInputLayerGPU(std::vector<std::vector<cv::cuda::GpuMat> > *input_batch)
{
	Blob<float>* input_layer = m_pNet->input_blobs()[0];

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

std::vector<vector<Prediction> > _Caffe::ClassifyBatchGPU(const vector<cv::cuda::GpuMat> imgs, int num_classes)
{
	vector<float> output_batch = PredictBatchGPU(imgs);

	vector<vector<Prediction> > predictions;
	Blob<float>* output_layer = m_pNet->output_blobs()[0];

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
			prediction_single.push_back(make_pair(m_vLabel[idx], output[idx]));
		}

		predictions.push_back(vector<Prediction>(prediction_single));
	}
	return predictions;
}

std::vector<float> _Caffe::PredictBatchGPU(const vector<cv::cuda::GpuMat> imgs)
{
	Blob<float>* input_layer = m_pNet->input_blobs()[0];

	input_layer->Reshape(m_batchSize, m_nChannel, m_inputGeometry.height, m_inputGeometry.width);

	/* Forward dimension change to all layers. */
	m_pNet->Reshape();

	std::vector<std::vector<cv::cuda::GpuMat> > input_batch;
	WrapBatchInputLayerGPU(&input_batch);

	PreprocessBatchGPU(imgs, &input_batch);

	m_pNet->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = m_pNet->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels() * imgs.size();
	return std::vector<float>(begin, end);
}



void _Caffe::PreprocessBatchGPU(const vector<cv::cuda::GpuMat> imgs, std::vector<std::vector<cv::cuda::GpuMat> >* input_batch)
{
	for (int i = 0; i < imgs.size(); i++)
	{
		cv::cuda::GpuMat img = imgs[i];
		std::vector<cv::cuda::GpuMat> *input_channels = &(input_batch->at(i));

		/* Convert the input image to the input image format of the network. */
		cv::cuda::GpuMat sample;
		if (img.channels() == 3 && m_nChannel == 1)
			cv::cuda::cvtColor(img, sample, CV_BGR2GRAY);
		else if (img.channels() == 4 && m_nChannel == 1)
			cv::cuda::cvtColor(img, sample, CV_BGRA2GRAY);
		else if (img.channels() == 4 && m_nChannel == 3)
			cv::cuda::cvtColor(img, sample, CV_BGRA2BGR);
		else if (img.channels() == 1 && m_nChannel == 3)
			cv::cuda::cvtColor(img, sample, CV_GRAY2BGR);
		else
			sample = img;

		cv::cuda::GpuMat sample_resized;
		if (sample.size() != m_inputGeometry)
			cv::cuda::resize(sample, sample_resized, m_inputGeometry);
		else
			sample_resized = sample;

		cv::cuda::GpuMat sample_float;
		if (m_nChannel == 3)
			sample_resized.convertTo(sample_float, CV_32FC3);
		else
			sample_resized.convertTo(sample_float, CV_32FC1);

		cv::cuda::GpuMat sample_normalized;
		cv::cuda::subtract(sample_float, m_mMean, sample_normalized);

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

#endif
