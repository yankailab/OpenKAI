/*
 * _CaffeRegressionInf.h
 *
 *  Created on: May 23, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN__CaffeRegressionInf_H_
#define OpenKAI_src_DNN__CaffeRegressionInf_H_

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

	bool addImg(cv::Mat& img);
	double* getOutput(void);
	void reset(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CaffeRegressionInf*) This)->update();
		return NULL;
	}

	string m_baseDir;
	string m_fDeployProto;
	string m_fInfCaffemodel;
	string m_infLayerInput;

	int m_width;
	int m_height;
	int m_nChannel;
	int m_outputDim;
	vInt3 m_meanCol;

	int m_batchSize;
	int m_dataSize;
	int m_batchIter;

	float* m_pData;
	float* m_pDummy;

	double* m_pOutput;
	int		m_iImg;

};
}

#endif
#endif
