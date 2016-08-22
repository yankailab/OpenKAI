/*
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_SSD.h"

namespace kai
{
_SSD::_SSD()
{
	_ThreadBase();

	num_channels_ = 0;
	m_pUniverse = NULL;
	m_pCamStream = NULL;
	m_pFrame = NULL;
}

_SSD::~_SSD()
{
}

bool _SSD::init(JSON* pJson, string ssdName)
{
	//Setup Caffe Classifier
	string caffeDir = "";
	string modelFile;
	string trainedFile;
	string meanFile;
	string labelFile;

	CHECK_INFO(pJson->getVal("CAFFE_DIR", &caffeDir));
	CHECK_FATAL(pJson->getVal("CAFFE_SSD_MODEL_FILE", &modelFile));
	CHECK_FATAL(pJson->getVal("CAFFE_SSD_TRAINED_FILE", &trainedFile));
	CHECK_FATAL(pJson->getVal("CAFFE_SSD_MEAN_FILE", &meanFile));
	CHECK_FATAL(pJson->getVal("CAFFE_SSD_LABEL_FILE", &labelFile));

	setup(caffeDir + modelFile, caffeDir + trainedFile, caffeDir + meanFile,
			caffeDir + labelFile);
	LOG(INFO)<<"Caffe Initialized";

	double FPS = DEFAULT_FPS;
	CHECK_ERROR(pJson->getVal("CAFFE_SSD_FPS", &FPS));
	this->setTargetFPS(FPS);

	m_pFrame = new Frame();

	return true;
}

void _SSD::setup(const string& model_file, const string& trained_file,
		const string& mean_file, const string& label_file)
{

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
//	SetMean(mean_file);

//	/* Load labels. */
//	std::ifstream labels(label_file.c_str());
//	CHECK(labels) << "Unable to open labels file " << label_file;
//	string line;
//	while (std::getline(labels, line))
//		labels_.push_back(string(line));
//
//	Blob<float>* output_layer = net_->output_blobs()[0];
//	CHECK_EQ(labels_.size(), output_layer->channels())<< "Number of labels is different from the output layer dimension.";

}

bool _SSD::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _SSD::update(void)
{
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif


	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_frameID = get_time_usec();

		detectFrame();

		this->autoFPSto();
	}

}

void _SSD::detectFrame(void)
{
	string name;
	Rect bb;
	Frame* pFrame;

	if (m_pCamStream == NULL)
		return;
	if (m_pUniverse == NULL)
		return;

	pFrame = m_pCamStream->getFrame();
	if (pFrame->empty())return;
	if (!pFrame->isNewerThan(m_pFrame))return;
	m_pFrame->update(pFrame);

	Mat* pImg = m_pFrame->getCMat();
	std::vector<vector<float> > detections = detect(*pImg);

	float confidence_threshold = 0.1;

	/* Print the detection results. */
	for (int i = 0; i < detections.size(); ++i)
	{
		const vector<float>& d = detections[i];
		// Detection format: [image_id, label, score, xmin, ymin, xmax, ymax].
//		CHECK_EQ(d.size(), 7);

		float size = d.size();

		const float score = d[2];
		if (score < confidence_threshold)
			continue;

		name = static_cast<int>(d[1]);
		bb.x = d[3] * pImg->cols;
		bb.y = d[4] * pImg->rows;
		bb.width = d[5] * pImg->cols - bb.x;
		bb.height = d[6] * pImg->rows - bb.y;

		m_pUniverse->addKnownObject(name, NULL, &bb, NULL);

	}

}

/* Load the mean file in binaryproto format. */
void _SSD::SetMean(const string& mean_file)
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

std::vector<vector<float> > _SSD::detect(const cv::Mat img)
{
	Blob<float>* input_layer = net_->input_blobs()[0];
	input_layer->Reshape(1, num_channels_, input_geometry_.height,
			input_geometry_.width);
	/* Forward dimension change to all layers. */
	net_->Reshape();

	std::vector<cv::Mat> input_channels;
	WrapInputLayer(&input_channels);

	Preprocess(img, &input_channels);

	net_->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* result_blob = net_->output_blobs()[0];
	const float* result = result_blob->cpu_data();
	const int num_det = result_blob->height();
	vector<vector<float> > detections;
	for (int k = 0; k < num_det; ++k)
	{
		if (result[0] == -1)
		{
			// Skip invalid detection.
			result += 7;
			continue;
		}
		vector<float> detection(result, result + 7);
		detections.push_back(detection);
		result += 7;
	}

	return detections;
}

void _SSD::WrapInputLayer(std::vector<cv::Mat>* input_channels)
{
	Blob<float>* input_layer = net_->input_blobs()[0];

	int width = input_layer->width();
	int height = input_layer->height();
	float* input_data = input_layer->mutable_cpu_data();
	for (int i = 0; i < input_layer->channels(); ++i)
	{
		cv::Mat channel(height, width, CV_32FC1, input_data);
		input_channels->push_back(channel);
		input_data += width * height;
	}
}

void _SSD::Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels)
{
	/* Convert the input image to the input image format of the network. */
	cv::Mat sample;
	if (img.channels() == 3 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 4 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
	else if (img.channels() == 4 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
	else if (img.channels() == 1 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
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
//	cv::subtract(sample_float, mean_, sample_normalized);

	sample_normalized = sample_float;

	/* This operation will write the separate BGR planes directly to the
	 * input layer of the network because it is wrapped by the cv::Mat
	 * objects in input_channels. */
	cv::split(sample_normalized, *input_channels);

	CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
			== net_->input_blobs()[0]->cpu_data())
															<< "Input channels are not wrapping the input layer of the network.";
}


bool _SSD::draw(Frame* pFrame, iVector4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	putText(*pFrame->getCMat(), "SSD FPS: " + f2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}


}
