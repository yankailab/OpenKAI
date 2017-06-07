/*
 * _CaffeRegressionTrain.h
 *
 *  Created on: May 23, 2017
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__CaffeRegressionTrain_H_
#define OPENKAI_SRC_DNN__CaffeRegressionTrain_H_

#include "../../Base/common.h"

#ifdef USE_CAFFE

#include <boost/smart_ptr.hpp>
#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/layers/memory_data_layer.hpp>

#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"

namespace kai
{
using namespace caffe;

class _CaffeRegressionTrain: public _ThreadBase
{
public:
	_CaffeRegressionTrain();
	~_CaffeRegressionTrain();

	bool init(void* pKiss);

private:
	void train();
	void inference();
	int readImgListToFloat(string fImgList, float *pData, float *pLabel, int nRead);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CaffeRegressionTrain*) This)->update();
		return NULL;
	}

	//common
	string m_baseDir;
	int m_width;
	int m_height;
	int m_nChannel;
	int m_outputDim;
	vInt3 m_meanCol;

	//train
	int m_nTrain;
	int m_nTest;
	double	m_kLabel;

	string m_fSolverProto;
	string m_fPretrainedCaffemodel;
	string m_fTrainImgList;
	string m_fTestImgList;

	string m_layerInTrain;
	string m_layerInTest;
	string m_layerLabelTrain;
	string m_layerLabelTest;

	//inference
	int m_nInfBatch;
	int m_nInf;
	int m_nInfBatchIter;

	double m_err;

	string m_fDeployProto;
	string m_fInfCaffemodel;
	string m_fInfImgList;
	string m_infResultDir;
	string m_infLayerInput;

};
}

#endif
#endif
