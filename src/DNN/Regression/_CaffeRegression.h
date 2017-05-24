/*
 * _CaffeRegression.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__CaffeRegression_H_
#define OPENKAI_SRC_DNN__CaffeRegression_H_

#include "../../Base/common.h"

#ifdef USE_CAFFE

#include "../../Base/_ThreadBase.h"
#include <boost/smart_ptr.hpp>
#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/layers/memory_data_layer.hpp>

namespace kai
{
using namespace caffe;
//using caffe::Caffe;
//using caffe::Net;
//using caffe::Blob;
//using caffe::shared_ptr;
//using caffe::vector;

class _CaffeRegression: public _ThreadBase
{
public:
	_CaffeRegression();
	~_CaffeRegression();

	bool init(void* pKiss);
	void train();
	void inference();

	vector<string> split(string str, char c);
	void readImgListToFloat(string list_path, float *data, float *label, int data_len);
	void readImgFileName(string path, string *infiles);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CaffeRegression*) This)->update();
		return NULL;
	}

	int m_width;
	int m_height;
	int m_nChannel;
	int m_targetDim;
	vInt3 m_meanCol;

	string m_mode;

	//train
	int m_dataSizeTrain;
	int m_dataSizeTest;

	string m_baseDir;
	string m_fSolverProto;
	string m_fPretrainedCaffemodel;
	string m_fTrainImgList;
	string m_fTestImgList;

	string m_layerInTrain;
	string m_layerInTest;
	string m_layerLabelTrain;
	string m_layerLabelTest;

	//inference
	int m_infBatchSize;
	int m_infDataSize;
	int m_batchIter;

	string m_fDeployProto;
	string m_fInfCaffemodel;
	string m_fInfImgList;
	string m_infResultDir;
	string m_infLayerInput;

};
}

#endif
#endif
