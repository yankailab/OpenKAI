/*
 * _Caffe.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Caffe__Caffe_H_
#define OpenKAI_src_DNN_Caffe__Caffe_H_

#include "../../Base/common.h"

#ifdef USE_CAFFE
#ifdef USE_CUDA

#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/blob.hpp>
#include <caffe/common.hpp>
#include <caffe/util/io.hpp>
#include <caffe/proto/caffe.pb.h>

#include "../../Base/_ObjectBase.h"
#include "../../Vision/_VisionBase.h"

#define N_EXT 16

namespace kai
{

using namespace caffe;
using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::shared_ptr;
using caffe::vector;
using std::string;
using namespace std;
using namespace cv;
using namespace cuda;

// Pair (index, confidence) representing a prediction
typedef pair<int, float> Prediction;

class _Caffe: public _DetectorBase
{
public:
	_Caffe();
	~_Caffe();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);

	bool setup(void);
	vector<vector<Prediction> > Classify(const vector<GpuMat> vImg);
	int getClassIdx(string& className);
	string getClassName(int iClass);

private:
	int getDirFileList(void);
	bool verifyExtension(string* fName);
	vector<int> batchInf(void);
	void detect(void);
	void SetMean(const string& meanFile);
	vector<float> Predict(const vector<GpuMat> vImg);
	void WrapInputLayer(vector<vector<GpuMat> > *vvInput);
	void Preprocess(const vector<GpuMat> vImg, vector<vector<GpuMat> >* vvInputBatch);

	void update(void);
	static void* getUpdateThread(void* This) {
		((_Caffe*) This)->update();
		return NULL;
	}

private:
	shared_ptr<Net<float> > m_pNet;
	cv::Size m_inputGeometry;
	int m_nChannel;
	GpuMat m_mMean;
	vector<string> m_vLabel;
	int m_nBatch;

	Frame* m_pBGR;
	Frame* m_pRGBA;

	string m_dirIn;
	vector<string> m_vExtIn;
	vector<string> m_vFileIn;
};
}

#endif
#endif
#endif
