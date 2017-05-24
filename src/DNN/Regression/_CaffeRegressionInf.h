/*
 * _CaffeRegressionInf.h
 *
 *  Created on: May 23, 2017
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__CaffeRegressionInf_H_
#define OPENKAI_SRC_DNN__CaffeRegressionInf_H_

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

class _CaffeRegressionInf: public _ThreadBase
{
public:
	_CaffeRegressionInf();
	~_CaffeRegressionInf();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void inference();
	void readImgListToFloat(string list_path, float *data, float *label, int data_len);
	void readImgFileName(string path, string *infiles);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CaffeRegressionInf*) This)->update();
		return NULL;
	}

	int m_width;
	int m_height;
	int m_nChannel;
	int m_targetDim;
	vInt3 m_meanCol;

	int m_infBatchSize;
	int m_infDataSize;
	int m_batchIter;

	string m_baseDir;
	string m_fDeployProto;
	string m_fInfCaffemodel;
	string m_fInfImgList;
	string m_infResultDir;
	string m_infLayerInput;

};
}

#endif
#endif
