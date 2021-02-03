/*
 * _Caffe.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_Caffe.h"

#ifdef USE_OPENCV
#ifdef USE_CAFFE
#ifdef USE_CUDA

namespace kai
{

_Caffe::_Caffe()
{
	m_nChannel = 0;
	m_nBatch = 1;
	m_pRGBA = NULL;
	m_pBGR = NULL;
	m_nClass = 0;

	m_dirIn = "";
}

_Caffe::~_Caffe()
{
}

bool _Caffe::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,nBatch);

	m_pBGR = new Frame();
	m_pRGBA = new Frame();

	KISSm(pK, dirIn);
	m_vExtIn.clear();
	string pExtIn[N_EXT];
	int nExt = pK->array("extIn", pExtIn, N_EXT);
	for(int i=0; i<nExt; i++)
	{
		m_vExtIn.push_back(pExtIn[i]);
	}

	return true;
}

void _Caffe::SetMean(const string& meanFile)
{
	BlobProto blobProto;
	ReadProtoFromBinaryFileOrDie(meanFile.c_str(), &blobProto);

	// Convert from BlobProto to Blob<float>
	Blob<float> meanBlob;
	meanBlob.FromProto(blobProto);

	CHECK_EQ(meanBlob.channels(), m_nChannel)<< "Number of channels of mean file doesn't match input layer";

	// The format of the mean file is planar 32-bit float BGR or grayscale
	std::vector<cv::Mat> channels;
	float* data = meanBlob.mutable_cpu_data();

	for (int i = 0; i < m_nChannel; i++)
	{
		// Extract an individual channel
		cv::Mat channel(meanBlob.height(), meanBlob.width(), CV_32FC1, data);
		channels.push_back(channel);
		data += meanBlob.height() * meanBlob.width();
	}

	// Merge the separate channels into a single image
	cv::Mat mean;
	cv::merge(channels, mean);

	// Compute the global mean pixel value and create a mean image filled with this value
	cv::Scalar channelMean = cv::mean(mean);
	cv::Mat mMean = cv::Mat(m_inputGeometry, mean.type(), channelMean);
	m_mMean.upload(mMean);
}

bool _Caffe::setup(void)
{
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	m_pNet.reset(new Net<float>(m_fModel, TEST));
	m_pNet->CopyTrainedLayersFrom(m_fWeight);

	CHECK_EQ(m_pNet->num_inputs(), 1)<< "Network should have exactly one input";
	CHECK_EQ(m_pNet->num_outputs(), 1)<< "Network should have exactly one output";

	Blob<float>* inputLayer = m_pNet->input_blobs()[0];
	m_nChannel = inputLayer->channels();
	CHECK(m_nChannel == 3 || m_nChannel == 1)<< "Input layer should have 1 or 3 channels";
	m_inputGeometry = cv::Size(inputLayer->width(), inputLayer->height());

	// Load the binaryproto mean file
	SetMean(m_fMean);

	// Load labels
	std::ifstream labels(m_labelFile.c_str());
	CHECK(labels) << "Unable to open labels file " << m_labelFile;
	string line;
	while (std::getline(labels, line))
		m_vLabel.push_back(string(line));

	Blob<float>* outputLayer = m_pNet->output_blobs()[0];
	CHECK_EQ(m_vLabel.size(), outputLayer->channels())<< "Number of labels is different from the output layer dimension";

	m_nClass = m_vLabel.size();

	return true;
}

bool _Caffe::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Caffe::update(void)
{
	IF_(!setup());
	m_bReady = true;
	IF_(m_mode == noThread);

	if(m_mode == batch)
	{
		batchInf();
		return;
	}

	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		detect();

		m_pT->autoFPSto();
	}
}

vector<int> _Caffe::batchInf(void)
{
	vector <int> vResult;

	if(getDirFileList()<=0)
	{
		return vResult;
	}

	int nTot = 0;
	double progress = 0.0;

	for(int i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		GpuMat gImg;
		gImg.upload(mIn);

		m_pBGR->update(&gImg);
		m_pRGBA->getRGBAOf(m_pBGR);
		GpuMat gRGBA = *m_pRGBA->getGMat();
		IF_CONT(gRGBA.empty());

		vector<GpuMat> vBatch;
		vBatch.clear();
		vBatch.push_back(gRGBA);
		std::vector<vector<Prediction> > vPred;
		vPred.clear();

		vPred = Classify(vBatch);

		for(int i=0; i<vBatch.size(); i++)
		{
			vector<Prediction> vP = vPred[i];
			vResult.push_back(vP[0].first);
		}

		nTot++;
		double prog = (double)i/(double)m_vFileIn.size();
		if(prog - progress > 0.1)
		{
			progress = prog;
			LOG_I("Inference: " << (int)(progress * 100) << "%");
		}
	}

	LOG_I("Total inferred: " << nTot);

	return vResult;
}

int _Caffe::getDirFileList(void)
{
	m_vFileIn.clear();

	if (m_dirIn.at(m_dirIn.length() - 1) != '/')
		m_dirIn.push_back('/');

	DIR* pDirIn = opendir(m_dirIn.c_str());

	if (!pDirIn)
	{
		LOG_E("Input directory not found");
		return -1;
	}

	struct dirent *dir;
	ifstream ifs;

	while ((dir = readdir(pDirIn)) != NULL)
	{
		string fileIn = m_dirIn + dir->d_name;

		IF_CONT(!verifyExtension(&fileIn));
		ifs.open(fileIn.c_str(), std::ios::in);
		IF_CONT(!ifs);
		ifs.close();

		m_vFileIn.push_back(dir->d_name);
	}

	closedir(pDirIn);

	return m_vFileIn.size();
}

bool _Caffe::verifyExtension(string* fName)
{
	NULL_F(fName);

	int i;
	for(i=0; i<m_vExtIn.size(); i++)
	{
		string ext = m_vExtIn[i];
		size_t extPos = fName->find(ext);
		IF_CONT(extPos == std::string::npos);
		IF_CONT(fName->substr(extPos) != ext);

		return true;
	}

	return false;
}

void _Caffe::detect(void)
{
	IF_(!m_bActive);
	NULL_(m_pVision);
	Frame* pBGR = m_pVision->BGR();
	NULL_(pBGR);
	IF_(pBGR->bEmpty());

	*m_pRGBA = pBGR->rgba();
	GpuMat gRGBA = *m_pRGBA->getGMat();
	IF_(gRGBA.empty());

	OBJECT* pO;
	Rect bb;
	vector<GpuMat> vBatch;
	vector<vector<GpuMat> > vvBatch;
	vBatch.clear();
	vvBatch.clear();
	int i,j,k;

	for(i=0; i<m_obj.size(); i++)
	{
		pO = m_obj.at(i);
		pO->m_camSize.x = gRGBA.cols;
		pO->m_camSize.y = gRGBA.rows;
		pO->f2iBBox();
		IF_CONT(pO->m_bbox.area() <= 0);

		vInt42rect(&pO->m_bbox, &bb);
		vBatch.push_back(GpuMat(gRGBA, bb));

		IF_CONT(vBatch.size() < m_nBatch);

		vvBatch.push_back(vBatch);
		vBatch.clear();
	}

	if(vBatch.size()>0)
	{
		vvBatch.push_back(vBatch);
		vBatch.clear();
	}

	std::vector<vector<Prediction> > vPred;
	vPred.clear();
	k=0;

	for(i=0; i<vvBatch.size(); i++)
	{
		vBatch = vvBatch[i];
		vPred = Classify(vBatch);

		for(j=0; j<vBatch.size(); j++)
		{
			vector<Prediction> vP = vPred[j];
			pO = m_obj.at(k++);

			pO->resetClass();
			pO->setTopClass(vP[0].first);
		}
	}
}

static bool PairCompare(const std::pair<float, int>& lhs, const std::pair<float, int>& rhs)
{
	return lhs.first > rhs.first;
}

// Return the indices of the top N values of vector v
static std::vector<int> Argmax(const std::vector<float>& v, int N)
{
	std::vector<std::pair<float, int> > pairs;
	for (size_t i = 0; i < v.size(); ++i)
		pairs.push_back(std::make_pair(v[i], i));
	std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

	std::vector<int> result;
	for (int i = 0; i < N; ++i)
		result.push_back(pairs[i].second);

	return result;
}

std::vector<vector<Prediction> > _Caffe::Classify(const vector<GpuMat> vImg)
{
	vector<float> vOutput = Predict(vImg);

	vector<vector<Prediction> > vvPrediction;
	Blob<float>* outputLayer = m_pNet->output_blobs()[0];

	for (int j = 0; j < vImg.size(); j++)
	{
		vector<float> output(
				vOutput.begin() + j * outputLayer->channels(),
				vOutput.begin() + (j + 1) * outputLayer->channels());

		vector<int> vMaxN = Argmax(output, m_nClass);
		vector<Prediction> vPrediction;

		for (int i = 0; i < m_nClass; i++)
		{
			int idx = vMaxN[i];
			vPrediction.push_back(make_pair(idx, output[idx]));
		}

		vvPrediction.push_back(vector<Prediction>(vPrediction));
	}

	return vvPrediction;
}

std::vector<float> _Caffe::Predict(const vector<GpuMat> vImg)
{
	Blob<float>* inputLayer = m_pNet->input_blobs()[0];
	inputLayer->Reshape(m_nBatch, m_nChannel, m_inputGeometry.height, m_inputGeometry.width);

	// Forward dimension change to all layers
	m_pNet->Reshape();

	std::vector<std::vector<GpuMat> > vvInput;
	WrapInputLayer(&vvInput);
	Preprocess(vImg, &vvInput);

	m_pNet->Forward();

	// Copy the output layer to a std::vector
	Blob<float>* outputLayer = m_pNet->output_blobs()[0];
	const float* begin = outputLayer->cpu_data();
	const float* end = begin + outputLayer->channels() * vImg.size();
	return std::vector<float>(begin, end);
}

void _Caffe::WrapInputLayer(std::vector<std::vector<GpuMat> > *vvInput)
{
	Blob<float>* inputLayer = m_pNet->input_blobs()[0];

	int w = inputLayer->width();
	int h = inputLayer->height();
	int n = inputLayer->num();
	float* pInputData = inputLayer->mutable_gpu_data();

	for (int j = 0; j < n; j++)
	{
		vector<GpuMat> vInputChannel;
		for (int i = 0; i < inputLayer->channels(); ++i)
		{
			GpuMat channel(h, w, CV_32FC1, pInputData);
			vInputChannel.push_back(channel);
			pInputData += w * h;
		}
		vvInput->push_back(vector<GpuMat>(vInputChannel));
	}
}

void _Caffe::Preprocess(const vector<GpuMat> vImg, std::vector<std::vector<GpuMat> >* vvInputBatch)
{
	for (int i = 0; i < vImg.size(); i++)
	{
		GpuMat img = vImg[i];
		std::vector<GpuMat> *input_channels = &(vvInputBatch->at(i));

		// Convert the input image to the input image format of the network
		GpuMat sample;
		if (img.channels() == 3 && m_nChannel == 1)
			cuda::cvtColor(img, sample, CV_BGR2GRAY);
		else if (img.channels() == 4 && m_nChannel == 1)
			cuda::cvtColor(img, sample, CV_BGRA2GRAY);
		else if (img.channels() == 4 && m_nChannel == 3)
			cuda::cvtColor(img, sample, CV_BGRA2BGR);
		else if (img.channels() == 1 && m_nChannel == 3)
			cuda::cvtColor(img, sample, CV_GRAY2BGR);
		else
			sample = img;

		GpuMat sample_resized;
		if (sample.size() != m_inputGeometry)
			cuda::resize(sample, sample_resized, m_inputGeometry);
		else
			sample_resized = sample;

		GpuMat sample_float;
		if (m_nChannel == 3)
			sample_resized.convertTo(sample_float, CV_32FC3);
		else
			sample_resized.convertTo(sample_float, CV_32FC1);

		GpuMat sample_normalized = sample_float;
//		cuda::subtract(sample_float, m_mMean, sample_normalized);

		/* This operation will write the separate BGR planes directly to the
		 * input layer of the network because it is wrapped by the cv::Mat
		 * objects in input_channels. */
		cuda::split(sample_normalized, *input_channels);
	}
}

int _Caffe::getClassIdx(string& className)
{
	int i;
	for(i=0; i<m_nClass; i++)
	{
		if(m_vLabel[i] == className)
			return i;
	}

	return -1;
}

string _Caffe::getClassName(int iClass)
{
	string className = m_vLabel[iClass];
	return className;
}

bool _Caffe::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	return true;
}

}
#endif
#endif
#endif
