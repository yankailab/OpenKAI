/*
 * _CaffeRegressionInf.cpp
 *
 *  Created on: May 23, 2017
 *      Author: yankai
 */

#include "_CaffeRegressionInf.h"

#ifdef USE_CAFFE

namespace kai
{
_CaffeRegressionInf::_CaffeRegressionInf()
{
	m_baseDir = "";
	m_width = 224;
	m_height = 224;
	m_nChannel = 3;
	m_targetDim = 6;
	m_meanCol.init();

	m_infBatchSize = 1;
	m_infDataSize = 1000;
	m_batchIter = m_infDataSize / m_infBatchSize;

	m_fDeployProto = "";
	m_fInfCaffemodel = "";
	m_fInfImgList = "";
	m_infResultDir = "result";
	m_infLayerInput = "data";

}

_CaffeRegressionInf::~_CaffeRegressionInf()
{
}

bool _CaffeRegressionInf::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,baseDir);
	KISSm(pK,width);
	KISSm(pK,height);
	KISSm(pK,nChannel);
	KISSm(pK,targetDim);

	KISSm(pK,fDeployProto);
	KISSm(pK,fDeployProto);
	KISSm(pK,fInfCaffemodel);
	KISSm(pK,fInfImgList);
	KISSm(pK,infResultDir);

	KISSm(pK,infBatchSize);
	KISSm(pK,infDataSize);
	KISSm(pK,infLayerInput);

	m_fDeployProto = m_baseDir + m_fDeployProto;
	m_fInfCaffemodel = m_baseDir + m_fInfCaffemodel;
	m_fInfImgList = m_baseDir + m_fInfImgList;

	int pMeanCol[3];
	if(pK->array("meanCol", pMeanCol, 3))
	{
		m_meanCol.x = pMeanCol[0];
		m_meanCol.y = pMeanCol[1];
		m_meanCol.z = pMeanCol[2];
	}

	return true;
}

bool _CaffeRegressionInf::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _CaffeRegressionInf::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _CaffeRegressionInf::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->autoFPSto();
	}
}

void _CaffeRegressionInf::inference()
{
	Caffe::set_mode(Caffe::GPU);

	m_batchIter = m_infDataSize / m_infBatchSize;

	string *infiles = new string[m_infDataSize];
	readImgFileName(m_fInfImgList.c_str(), infiles);

	float* pInfData = new float[m_infDataSize * m_height * m_width * m_nChannel];
	float* pInfLabel = new float[m_infDataSize * m_targetDim];

	Net<float> infNet(m_fDeployProto.c_str(), TEST);
	infNet.CopyTrainedLayersFrom(m_fInfCaffemodel.c_str());

	readImgListToFloat(m_fInfImgList.c_str(), pInfData, pInfLabel, m_infDataSize);

	const auto pInfInputlayer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(infNet.layer_by_name(m_infLayerInput.c_str()));
	float *pInfDummy = new float[m_infDataSize];
	for (int i = 0; i < m_infDataSize; i++)
		pInfDummy[i] = 0.0f;

	cv::Mat oImg;
	cv::Mat resizedOimg;

	for (int iBatch = 0; iBatch < m_batchIter; iBatch++)
	{
		pInfInputlayer->Reset((float*) pInfData + iBatch * m_width * m_height * m_nChannel * m_infBatchSize,
				pInfDummy + iBatch * m_infBatchSize, m_infBatchSize);
		const auto result = infNet.Forward();

		const auto data = result[1]->cpu_data();
		for (int i = 0; i < m_infBatchSize; i++)
		{
			int totalID = iBatch * m_infBatchSize + i;
			string fName = m_baseDir + infiles[totalID];
			oImg = imread(fName);
			cv::resize(oImg, resizedOimg, cv::Size(m_width, m_height));

			int *val = new int(m_targetDim);
			for (int j = 0; j < m_targetDim; j++)
			{
				val[j] = (int) (data[i * m_targetDim + j] * 256);
				if (val[j] < 0)
					val[j] = 0;
				if (j == 0 && val[j] > m_width)
					val[j] = m_width - 1;
				if (j == 1 && val[j] > m_height)
					val[j] = m_height - 1;
				if (j >= 3 && val[j] > 255)
					val[j] = 255;
			}

			circle(resizedOimg, cv::Point(val[0], val[1]), val[2], cv::Scalar(val[3], val[4], val[5]), 3);

			stringstream ofname;
			ofname	<< m_baseDir << m_infResultDir << setw(5) << setfill('0') << i2str(totalID) << ".png";
			imwrite(ofname.str(), resizedOimg);
		}
	}

	oImg.release();
	resizedOimg.release();
	delete[] pInfData;
	delete[] pInfLabel;
	delete[] pInfDummy;
	delete[] infiles;
}

void _CaffeRegressionInf::readImgListToFloat(string list_path, float *data, float *label, int data_len)
{
	ifstream ifs;
	string str;
	int n = 0;
	ifs.open(list_path.c_str(), std::ios::in);
	if (!ifs)
	{
		LOG(INFO)<< "cannot open " << list_path; return;
	}

	while (getline(ifs, str))
	{
		vector<string> entry = splitBy(str, '\t');
		string fName = m_baseDir + entry[0];
		cout << "reading: " << fName << endl;
		cv::Mat img = cv::imread(fName);
		cv::Mat resized_img;
		cv::resize(img, resized_img, cv::Size(m_width, m_height));

		for (int y = 0; y < m_height; y++)
		{
			for (int x = 0; x < m_width; x++)
			{
				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 0
						+ m_width * m_height * m_nChannel * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 0] - m_meanCol.x;

				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 1
						+ m_width * m_height * m_nChannel * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 1] - m_meanCol.y;

				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 2
						+ m_width * m_height * m_nChannel * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 2] - m_meanCol.z;
			}
		}

		for (int i = 0; i < m_targetDim; i++)
		{
			label[n * m_targetDim + i] = stof(entry[i + 1]) / 256.0;
		}
		n++;
	}
}

void _CaffeRegressionInf::readImgFileName(string path, string *infiles)
{
	ifstream ifs;
	ifs.open(path.c_str(), ios::in);
	string str;

	int n = 0;
	while (getline(ifs, str))
	{
		vector<string> entry = splitBy(str, '\t');
		infiles[n] = entry[0];
		n++;
	}

	ifs.close();
}


}

#endif
