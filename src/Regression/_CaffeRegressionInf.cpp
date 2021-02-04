/*
 * _CaffeRegressionInf.cpp
 *
 *  Created on: May 23, 2017
 *      Author: yankai
 */

#include "_CaffeRegressionInf.h"

#ifdef USE_OPENCV
#ifdef USE_CAFFE
#ifdef USE_CUDA

namespace kai
{
_CaffeRegressionInf::_CaffeRegressionInf()
{
	m_baseDir = "";
	m_width = 224;
	m_height = 224;
	m_nChannel = 3;
	m_outputDim = 6;
	m_meanCol.init();

	m_batchSize = 1;
	m_dataSize = 1000;
	m_batchIter = m_dataSize / m_batchSize;

	m_fDeployProto = "";
	m_fInfCaffemodel = "";
	m_infLayerInput = "data";

	m_pData = NULL;
	m_pDummy = NULL;
	m_pOutput = NULL;
	m_iImg = 0;

}

_CaffeRegressionInf::~_CaffeRegressionInf()
{
	DEL(m_pData);
	DEL(m_pDummy);
	DEL(m_pOutput);
}

bool _CaffeRegressionInf::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, baseDir);
	KISSm(pK, width);
	KISSm(pK, height);
	KISSm(pK, nChannel);
	KISSm(pK, outputDim);

	KISSm(pK, fDeployProto);
	KISSm(pK, fDeployProto);
	KISSm(pK, fInfCaffemodel);

	KISSm(pK, batchSize);
	KISSm(pK, dataSize);
	KISSm(pK, infLayerInput);

	m_fDeployProto = m_baseDir + m_fDeployProto;
	m_fInfCaffemodel = m_baseDir + m_fInfCaffemodel;

	int pMeanCol[3];
	if (pK->array("meanCol", pMeanCol, 3))
	{
		m_meanCol.x = pMeanCol[0];
		m_meanCol.y = pMeanCol[1];
		m_meanCol.z = pMeanCol[2];
	}

	return true;
}

bool _CaffeRegressionInf::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _CaffeRegressionInf::update(void)
{
	Caffe::set_mode(Caffe::GPU);

	//load net and layer
	Net<float> infNet(m_fDeployProto.c_str(), TEST);
	infNet.CopyTrainedLayersFrom(m_fInfCaffemodel.c_str());
	const auto pInputlayer =
			boost::dynamic_pointer_cast<MemoryDataLayer<float>>(
					infNet.layer_by_name(m_infLayerInput.c_str()));

	//prepare buf
	m_pData = new float[m_dataSize * m_height * m_width * m_nChannel];
	m_pDummy = new float[m_dataSize];
	for (int i = 0; i < m_dataSize; i++)
		m_pDummy[i] = 0.0f;
	m_batchIter = m_dataSize / m_batchSize;
	m_pOutput = new double[m_dataSize * m_outputDim];

	//start update loop
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		//start inference
		for (int iBatch = 0; iBatch < m_batchIter; iBatch++)
		{
			pInputlayer->Reset(
					(float*) m_pData
							+ iBatch * m_width * m_height * m_nChannel
									* m_batchSize,
					m_pDummy + iBatch * m_batchSize, m_batchSize);

			const auto pResult = infNet.Forward();
			const auto pResultData = pResult[1]->cpu_data();

			for (int i = 0; i < m_batchSize; i++)
			{
				int iOutput = iBatch * m_batchSize + i * m_outputDim;

				for (int j = 0; j < m_outputDim; j++)
				{
					m_pOutput[iOutput + j] = (double) (pResultData[i * m_outputDim + j]);
				}
			}
		}

		reset();

		m_pT->autoFPSto();
	}
}

bool _CaffeRegressionInf::addImg(cv::Mat& img)
{
	IF_F(m_iImg >= m_dataSize);

	cv::Mat rImg;
	cv::resize(img, rImg, cv::Size(m_width, m_height));

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			m_pData[y * rImg.cols + x + rImg.cols * rImg.rows * 0
					+ m_width * m_height * m_nChannel * m_iImg]
					=
					rImg.data[y * rImg.step + x * rImg.elemSize() + 0] - m_meanCol.x;

			m_pData[y * rImg.cols + x + rImg.cols * rImg.rows * 1
					+ m_width * m_height * m_nChannel * m_iImg]
					=
					rImg.data[y * rImg.step + x * rImg.elemSize() + 1] - m_meanCol.y;

			m_pData[y * rImg.cols + x + rImg.cols * rImg.rows * 2
					+ m_width * m_height * m_nChannel * m_iImg]
					=
					rImg.data[y * rImg.step + x * rImg.elemSize() + 2] - m_meanCol.z;
		}
	}

	m_iImg++;

	return true;
}

double* _CaffeRegressionInf::getOutput(void)
{
	return m_pOutput;
}


}
#endif
#endif
#endif
