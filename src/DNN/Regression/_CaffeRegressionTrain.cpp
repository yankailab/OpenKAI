/*
 * _CaffeRegressionTrain.cpp
 *
 *  Created on: May 23, 2017
 *      Author: yankai
 */

#include "_CaffeRegressionTrain.h"

#ifdef USE_CAFFE

namespace kai
{
_CaffeRegressionTrain::_CaffeRegressionTrain()
{
	m_baseDir = "";
	m_width = 224;
	m_height = 224;
	m_nChannel = 3;
	m_outputDim = 6;
	m_kLabel = 1.0;
	m_meanCol.init();

	m_fSolverProto = "";
	m_fPretrainedCaffemodel = "";
	m_fTrainImgList = "";
	m_fTestImgList = "";

	m_layerInTrain = "data";
	m_layerInTest = "data";
	m_layerLabelTrain = "label";
	m_layerLabelTest = "label";

	m_dataSizeTrain = 1000;
	m_dataSizeTest = 1000;

	m_infBatchSize = 1;
	m_infDataSize = 1000;
	m_infBatchIter = m_infDataSize / m_infBatchSize;

	m_fDeployProto = "";
	m_fInfCaffemodel = "";
	m_fInfImgList = "";
	m_infResultDir = "result";
	m_infLayerInput = "data";
}

_CaffeRegressionTrain::~_CaffeRegressionTrain()
{
}

bool _CaffeRegressionTrain::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,baseDir);

	KISSm(pK,fSolverProto);
	KISSm(pK,fPretrainedCaffemodel);
	KISSm(pK,fTrainImgList);
	KISSm(pK,fTestImgList);

	KISSm(pK,width);
	KISSm(pK,height);
	KISSm(pK,nChannel);
	KISSm(pK,outputDim);
	KISSm(pK,kLabel);

	KISSm(pK,layerInTrain);
	KISSm(pK,layerInTest);
	KISSm(pK,layerLabelTrain);
	KISSm(pK,layerLabelTest);

	KISSm(pK,dataSizeTrain);
	KISSm(pK,dataSizeTest);

	m_fSolverProto = m_baseDir + m_fSolverProto;
	m_fTrainImgList = m_baseDir + m_fTrainImgList;
	m_fTestImgList = m_baseDir + m_fTestImgList;
	if(!m_fPretrainedCaffemodel.empty())
	{
		m_fPretrainedCaffemodel = m_baseDir + m_fPretrainedCaffemodel;
	}

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

	train();

	exit(0);
	return true;
}

void _CaffeRegressionTrain::train()
{
	Caffe::set_mode(Caffe::GPU);

	//read prototxt
	SolverParameter solver_param;
	ReadProtoFromTextFileOrDie(m_fSolverProto.c_str(), &solver_param);
	std::shared_ptr<Solver<float>> solver(SolverRegistry<float>::CreateSolver(solver_param));

	const auto pNet = solver->net();
	const auto pTestNet = solver->test_nets();

	if(!m_fPretrainedCaffemodel.empty())
	{
		pNet->CopyTrainedLayersFrom(m_fPretrainedCaffemodel.c_str());
		pTestNet[0]->CopyTrainedLayersFrom(m_fPretrainedCaffemodel.c_str());
	}

	//read data
	float* pDataInTrain = new float[m_dataSizeTrain * m_height * m_width * m_nChannel];
	float* pLabelTrain = new float[m_dataSizeTrain * m_outputDim];
	float* pDataInTest = new float[m_dataSizeTest * m_height * m_width * m_nChannel];
	float* pLabelTest = new float[m_dataSizeTest * m_outputDim];

	int nImgTrain = readImgListToFloat(m_fTrainImgList.c_str(), pDataInTrain, pLabelTrain);
	int nImgTest = readImgListToFloat(m_fTestImgList.c_str(), pDataInTest, pLabelTest);

	if(nImgTrain > m_dataSizeTrain)
	{
		LOG_E("nImgTrain > m_dataSizeTrain");
		return;
	}

	if(nImgTest > m_dataSizeTest)
	{
		LOG_E("nImgTest > m_dataSizeTest");
		return;
	}

	float* pDummyDataTrain = new float[m_dataSizeTrain];
	float* pDummtDataTest = new float[m_dataSizeTest];
	for (int i = 0; i < m_dataSizeTrain; i++)
		pDummyDataTrain[i] = 0;
	for (int i = 0; i < m_dataSizeTest; i++)
		pDummtDataTest[i] = 0;

	//reset layers
	const auto pLayerInTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerInTrain.c_str()));
	const auto pLayerInTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerInTest.c_str()));

	pLayerInTrain->Reset((float*) pDataInTrain, (float*) pDummyDataTrain, m_dataSizeTrain);
	pLayerInTest->Reset((float*) pDataInTest, (float*) pDummtDataTest, m_dataSizeTest);

	const auto pLayerLabelTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerLabelTrain.c_str()));
	const auto pLayerLabelTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerLabelTest.c_str()));

	pLayerLabelTrain->Reset((float*) pLabelTrain, (float*) pDummyDataTrain, m_dataSizeTrain);
	pLayerLabelTest->Reset((float*) pLabelTest, (float*) pDummtDataTest, m_dataSizeTest);

	//start solve
	LOG_I("Solve start");
	solver->Solve();

	//complete
	delete[] pDataInTrain;
	delete[] pLabelTrain;
	delete[] pDummyDataTrain;
	delete[] pDataInTest;
	delete[] pLabelTest;
	delete[] pDummtDataTest;
}

void _CaffeRegressionTrain::inference()
{
	Caffe::set_mode(Caffe::GPU);

	//read net and layer
	Net<float> infNet(m_fDeployProto.c_str(), TEST);
	infNet.CopyTrainedLayersFrom(m_fInfCaffemodel.c_str());
	const auto pInfInputlayer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(infNet.layer_by_name(m_infLayerInput.c_str()));

	//read into buf
	string *infiles = new string[m_infDataSize];
	readImgFileName(m_fInfImgList.c_str(), infiles);

	float* pInfData = new float[m_infDataSize * m_height * m_width * m_nChannel];
	float* pInfLabel = new float[m_infDataSize * m_outputDim];

	int nImgInf = readImgListToFloat(m_fInfImgList.c_str(), pInfData, pInfLabel);
	if(nImgInf > m_infDataSize)
	{
		LOG_E("nImgInf > m_infDataSize");
		return;
	}

	float *pInfDummy = new float[m_infDataSize];
	for (int i = 0; i < m_infDataSize; i++)
		pInfDummy[i] = 0.0f;

	//start inference
	m_infBatchIter = m_infDataSize / m_infBatchSize;
	cv::Mat oImg;
	cv::Mat resizedOimg;

	for (int iBatch = 0; iBatch < m_infBatchIter; iBatch++)
	{
		pInfInputlayer->Reset(
				(float*) pInfData + iBatch * m_width * m_height * m_nChannel * m_infBatchSize,
				pInfDummy + iBatch * m_infBatchSize,
				m_infBatchSize);

		const auto pResult = infNet.Forward();
		const auto pResultData = pResult[1]->cpu_data();

		for (int i = 0; i < m_infBatchSize; i++)
		{
			int totalID = iBatch * m_infBatchSize + i;
			string fName = m_baseDir + infiles[totalID];
			oImg = imread(fName);
			cv::resize(oImg, resizedOimg, cv::Size(m_width, m_height));

			int *pVal = new int(m_outputDim);
			for (int j = 0; j < m_outputDim; j++)
			{
				pVal[j] = (int) (pResultData[i * m_outputDim + j] * 256);	//256 for normalization from int to float

				if (pVal[j] < 0)
					pVal[j] = 0;

				if (j == 0 && pVal[j] > m_width)
					pVal[j] = m_width - 1;

				if (j == 1 && pVal[j] > m_height)
					pVal[j] = m_height - 1;

				if (j >= 3 && pVal[j] > 255)
					pVal[j] = 255;
			}

			circle(resizedOimg, cv::Point(pVal[0], pVal[1]), pVal[2], cv::Scalar(pVal[3], pVal[4], pVal[5]), 3);

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

int _CaffeRegressionTrain::readImgListToFloat(string fImgList, float *pData, float *pLabel)
{
	ifstream ifs;
	string str;
	ifs.open(fImgList.c_str(), std::ios::in);
	if (!ifs)
	{
		LOG_E("Cannot open: " << fImgList);
		return 0;
	}

	int nImg = 0;
	while (getline(ifs, str))
	{
		vector<string> entry = splitBy(str, '\t');
		string fName = m_baseDir + entry[0];
		cout << "reading: " << fName << endl;

		cv::Mat img = cv::imread(fName);
		if(img.cols != m_width || img.rows != m_height)
		{
			cv::Mat rImg;
			cv::resize(img, rImg, cv::Size(m_width, m_height));
			img = rImg;
		}

		for (int y = 0; y < m_height; y++)
		{
			int iB = m_width * m_height * m_nChannel * nImg;

			for (int x = 0; x < m_width; x++)
			{
				int iA = y * img.cols + x + img.cols * img.rows;
				int iC = y * img.step + x * img.elemSize();

				pData[iA * 0 + iB] = img.data[iC + 0] - m_meanCol.x;
				pData[iA * 1 + iB] = img.data[iC + 1] - m_meanCol.y;
				pData[iA * 2 + iB] = img.data[iC + 2] - m_meanCol.z;
			}
		}

		for (int i = 0; i < m_outputDim; i++)
		{
			pLabel[nImg * m_outputDim + i] = stof(entry[i + 1]) * m_kLabel;
		}

		nImg++;
	}

	return nImg;
}

void _CaffeRegressionTrain::readImgFileName(string path, string *infiles)
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
