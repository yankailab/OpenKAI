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
	KISSm(pK,targetDim);

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

	float* pDataInTrain = new float[m_dataSizeTrain * m_height * m_width * m_nChannel];
	float* pLabelTrain = new float[m_dataSizeTrain * m_targetDim];
	float* pDataInTest = new float[m_dataSizeTest * m_height * m_width * m_nChannel];
	float* pLabelTest = new float[m_dataSizeTest * m_targetDim];

	readImgListToFloat(m_fTrainImgList.c_str(), pDataInTrain, pLabelTrain);
	readImgListToFloat(m_fTestImgList.c_str(), pDataInTest, pLabelTest);

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
		cv::Mat resized_img;
		cv::resize(img, resized_img, cv::Size(m_width, m_height));

		for (int y = 0; y < m_height; y++)
		{
			for (int x = 0; x < m_width; x++)
			{
				pData[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 0
						+ m_width * m_height * m_nChannel * nImg] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 0] - m_meanCol.x;

				pData[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 1
						+ m_width * m_height * m_nChannel * nImg] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 1] - m_meanCol.y;

				pData[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 2
						+ m_width * m_height * m_nChannel * nImg] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 2] - m_meanCol.z;
			}
		}

		for (int i = 0; i < m_targetDim; i++)
		{
			pLabel[nImg * m_targetDim + i] = stof(entry[i + 1]) / 256.0;
		}

		nImg++;
	}

	return nImg;
}

}

#endif
