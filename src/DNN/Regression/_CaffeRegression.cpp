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
	m_baseDir = "";
	m_fSolverProto = "";
	m_fPretrainedCaffemodel = "";
	m_fTrainImgList = "";
	m_fTestImgList = "";

	m_width = 224;
	m_height = 224;
	m_nChannel = 3;
	m_targetDim = 6;
	m_meanCol.init();

	m_layerInTrain = "data";
	m_layerInTest = "data";
	m_layerLabelTrain = "label";
	m_layerLabelTest = "label";

	m_dataSizeTrain = 1000;
	m_dataSizeTest = 1000;

	m_infBatchSize = 10;
	m_infDataSize = 1000;
	m_batchIter = m_infDataSize / m_infBatchSize;

	m_fDeployProto = "";
	m_fInfCaffemodel = "";
	m_fInfImgList = "";
	m_infLayerInput = "data";

	m_mode = "";

}

_CaffeRegression::~_CaffeRegression()
{
}

bool _CaffeRegression::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,mode);
	KISSm(pK,baseDir);

	//train
	KISSm(pK,fSolverProto);
	KISSm(pK,fPretrainedCaffemodel);
	KISSm(pK,fTrainImgList);
	KISSm(pK,fTestImgList);

	KISSm(pK,width);
	KISSm(pK,height);
	KISSm(pK,nChannel);
	KISSm(pK,targetDim);

	KISSm(pK,layerInTrain);
	KISSm(pK,layerInTest);
	KISSm(pK,layerLabelTrain);
	KISSm(pK,layerLabelTest);

	KISSm(pK,dataSizeTrain);
	KISSm(pK,dataSizeTest);

	//inference
	KISSm(pK,fDeployProto);
	KISSm(pK,fDeployProto);
	KISSm(pK,fInfCaffemodel);
	KISSm(pK,fInfImgList);

	KISSm(pK,infBatchSize);
	KISSm(pK,infDataSize);
	KISSm(pK,infLayerInput);

	//common
	m_fSolverProto = m_baseDir + m_fSolverProto;
	m_fPretrainedCaffemodel = m_baseDir + m_fPretrainedCaffemodel;
	m_fTrainImgList = m_baseDir + m_fTrainImgList;
	m_fTestImgList = m_baseDir + m_fTestImgList;

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

	if(m_mode == "train")
	{
		train();
	}
	else if(m_mode == "inference")
	{
		inference();
	}

	exit(0);
	return true;
}

void _CaffeRegression::train()
{
	Caffe::set_mode(Caffe::GPU);

	SolverParameter solver_param;
	ReadProtoFromTextFileOrDie(m_fSolverProto.c_str(), &solver_param);
	std::shared_ptr<Solver<float>> solver(SolverRegistry<float>::CreateSolver(solver_param));

	const auto pNet = solver->net();
	const auto pTestNet = solver->test_nets();

	pNet->CopyTrainedLayersFrom(m_fPretrainedCaffemodel.c_str());
	pTestNet[0]->CopyTrainedLayersFrom(m_fPretrainedCaffemodel.c_str());

	float* pDataInTrain = new float[m_dataSizeTrain * m_height * m_width * m_nChannel];
	float* pLabelTrain = new float[m_dataSizeTrain * m_targetDim];
	float* pDataInTest = new float[m_dataSizeTest * m_height * m_width * m_nChannel];
	float* pLabelTest = new float[m_dataSizeTest * m_targetDim];

	readImgListToFloat(m_fTrainImgList.c_str(), pDataInTrain, pLabelTrain, m_dataSizeTrain);
	readImgListToFloat(m_fTestImgList.c_str(), pDataInTest, pLabelTest, m_dataSizeTest);

	const auto pLayerInTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerInTrain.c_str()));
	const auto pLayerInTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerInTest.c_str()));

	float* pDummyDataTrain = new float[m_dataSizeTrain];
	float* pDummtDataTest = new float[m_dataSizeTest];
	for (int i = 0; i < m_dataSizeTrain; i++)
		pDummyDataTrain[i] = 0;
	for (int i = 0; i < m_dataSizeTest; i++)
		pDummtDataTest[i] = 0;

	pLayerInTrain->Reset((float*) pDataInTrain, (float*) pDummyDataTrain, m_dataSizeTrain);
	pLayerInTest->Reset((float*) pDataInTest, (float*) pDummtDataTest, m_dataSizeTest);

	const auto pLayerLabelTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerLabelTrain.c_str()));
	const auto pLayerLabelTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerLabelTest.c_str()));

	pLayerLabelTrain->Reset((float*) pLabelTrain, (float*) pDummyDataTrain, m_dataSizeTrain);
	pLayerLabelTest->Reset((float*) pLabelTest, (float*) pDummtDataTest, m_dataSizeTest);

	LOG_I("Solve start");
	solver->Solve();

	delete[] pDataInTrain;
	delete[] pLabelTrain;
	delete[] pDummyDataTrain;
	delete[] pDataInTest;
	delete[] pLabelTest;
	delete[] pDummtDataTest;
}

void _CaffeRegression::inference()
{
	Caffe::set_mode(Caffe::GPU);

	m_batchIter = m_infDataSize / m_infBatchSize;

	string *infiles = new string[m_infDataSize];
	readImgFileName(m_fInfImgList.c_str(), infiles);
//	ofstream ofs;
//	ofs.open("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\result.txt", ios::out);

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
			oImg = imread(infiles[totalID]);
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
			ofname	<< m_infResultDir << setw(5) << setfill('0') << i2str(totalID) << ".png";
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

vector<string> _CaffeRegression::split(string str, char c)
{
	vector<string> v;
	string buf = "";
	stringstream ss;

	ss << str;
	while (getline(ss, buf, c))
	{
		v.push_back(buf);
	}

	return v;
}

void _CaffeRegression::readImgListToFloat(string list_path, float *data, float *label, int data_len)
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
		vector<string> entry = split(str, '\t');
		cout << "reading: " << entry[0] << endl;
		cv::Mat img = cv::imread(entry[0]);
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

void _CaffeRegression::readImgFileName(string path, string *infiles)
{
	ifstream ifs;
	ifs.open(path.c_str(), ios::in);
	string str;

	int n = 0;
	while (getline(ifs, str))
	{
		vector<string> entry = split(str, '\t');
		infiles[n] = entry[0];
		n++;
	}

	ifs.close();
}


}

#endif
