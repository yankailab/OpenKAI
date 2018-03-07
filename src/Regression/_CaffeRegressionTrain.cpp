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

	m_nTrain = 1000;
	m_nTest = 1000;

	m_nInfBatch = 1;
	m_nInf = 1000;
	m_nInfBatchIter = m_nInf / m_nInfBatch;
	m_err = 0.0;

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

	KISSm(pK,nTrain);
	KISSm(pK,nTest);

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

	KISSm(pK,nInfBatch);
	KISSm(pK,nInf);
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
	float* pDataInTrain = new float[m_nTrain * m_height * m_width * m_nChannel];
	float* pLabelTrain = new float[m_nTrain * m_outputDim];
	float* pDataInTest = new float[m_nTest * m_height * m_width * m_nChannel];
	float* pLabelTest = new float[m_nTest * m_outputDim];

	int nImgTrain = readImgListToFloat(m_fTrainImgList.c_str(), pDataInTrain, pLabelTrain, m_nTrain);
	int nImgTest = readImgListToFloat(m_fTestImgList.c_str(), pDataInTest, pLabelTest, m_nTest);

	if(nImgTrain > m_nTrain)
	{
		LOG_E("nImgTrain > m_nTrain");
		return;
	}

	if(nImgTest > m_nTest)
	{
		LOG_E("nImgTest > m_nTest");
		return;
	}

	float* pDummyDataTrain = new float[m_nTrain];
	float* pDummtDataTest = new float[m_nTest];
	for (int i = 0; i < m_nTrain; i++)
		pDummyDataTrain[i] = 0;
	for (int i = 0; i < m_nTest; i++)
		pDummtDataTest[i] = 0;

	//reset layers
	const auto pLayerInTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerInTrain.c_str()));
	const auto pLayerInTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerInTest.c_str()));

	pLayerInTrain->Reset((float*) pDataInTrain, (float*) pDummyDataTrain, m_nTrain);
	pLayerInTest->Reset((float*) pDataInTest, (float*) pDummtDataTest, m_nTest);

	const auto pLayerLabelTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerLabelTrain.c_str()));
	const auto pLayerLabelTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerLabelTest.c_str()));

	pLayerLabelTrain->Reset((float*) pLabelTrain, (float*) pDummyDataTrain, m_nTrain);
	pLayerLabelTest->Reset((float*) pLabelTest, (float*) pDummtDataTest, m_nTest);

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

	float* pInfData = new float[m_nInf * m_height * m_width * m_nChannel];
	float* pInfLabel = new float[m_nInf * m_outputDim];

	int nImgInf = readImgListToFloat(m_fInfImgList.c_str(), pInfData, pInfLabel, m_nInf);
	if(nImgInf > m_nInf)
	{
		LOG_E("nImgInf > m_nInf");
		return;
	}

	float *pInfDummy = new float[m_nInf];
	for (int i = 0; i < m_nInf; i++)
		pInfDummy[i] = 0.0f;

	//start inference
	m_nInfBatchIter = m_nInf / m_nInfBatch;
	m_err = 0.0;

	for (int iBatch = 0; iBatch < m_nInfBatchIter; iBatch++)
	{
		pInfInputlayer->Reset(
				(float*) pInfData + iBatch * m_width * m_height * m_nChannel * m_nInfBatch,
				pInfDummy + iBatch * m_nInfBatch,
				m_nInfBatch);

		const auto pResult = infNet.Forward();
		const auto pResultData = pResult[1]->cpu_data();

		for (int i = 0; i < m_nInfBatch; i++)
		{
			int totalID = iBatch * m_nInfBatch + i;

			for (int j = 0; j < m_outputDim; j++)
			{
				double r = (double)(pResultData[i * m_outputDim + j]) / m_kLabel;
				double v = (double)pInfLabel[totalID * m_outputDim + j];
				m_err += abs(r-v);
			}
		}
	}

	m_err /= m_nInf;
	LOG_I("Inference avr err: " << m_err);

	delete[] pInfData;
	delete[] pInfLabel;
	delete[] pInfDummy;
}

int _CaffeRegressionTrain::readImgListToFloat(string fImgList, float *pData, float *pLabel, int nRead)
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
			pLabel[nImg * m_outputDim + i] = (float)(stof(entry[i + 1]) * m_kLabel);
		}

		nImg++;
		if(nImg >= nRead)
			return nImg;
	}

	return nImg;
}

}

#endif
