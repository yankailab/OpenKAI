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

#include <cuda_runtime.h>
#include <caffe/caffe.hpp">
//#include <caffe/layers/memory_data_layer.hpp>

#define WIDTH	224
#define HEIGHT	224
#define CHANNEL	3
#define TARGET_DIM	6

namespace kai
{
using namespace caffe;
using namespace std;
using namespace cv;
// NOLINT(build/namespaces)
//using caffe::Caffe;
//using caffe::Net;
//using caffe::Blob;
//using caffe::shared_ptr;
//using caffe::vector;
//using std::string;

class _CaffeRegression: public _ThreadBase
{
public:
	_CaffeRegression();
	~_CaffeRegression();

	bool init(JSON* pJson);
	void setup(const string& model_file, const string& trained_file,
			const string& mean_file, const string& label_file, int batch_size);

	void setModeGPU();

	vector<string> split(string str, char c);
	void readImgListToFloat(string list_path, float *data, float *label, int data_len);
	void readImgFileName(string path, string *infiles);
	void run_googlenet_train();
	void run_googlenet_test();

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CaffeRegression*) This)->update();
		return NULL;
	}

};
}

#endif
#endif
