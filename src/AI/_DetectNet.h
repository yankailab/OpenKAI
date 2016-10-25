/*
 * _DetectNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef AI__DetectNet_H_
#define AI__DetectNet_H_

#include "../Base/common.h"

#ifdef USE_TENSORRT

#include "../Base/cvplatform.h"
#include "../Base/_ThreadBase.h"
#include "../Navigation/_Universe.h"
#include "../Stream/_Stream.h"

#include <cuda_runtime.h>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cudaMappedMemory.h"
#include "cudaNormalize.h"
#include "detectNet.h"

namespace kai
{

using std::string;
using namespace std;
using namespace cv;

class _DetectNet: public _ThreadBase
{
public:
	_DetectNet();
	~_DetectNet();

	bool init(Config* pConfig);
	bool link(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);

	std::vector<vector<float> > detect(Frame* pFrame);
	bool start(void);

private:
	void detectFrame(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DetectNet*) This)->update();
		return NULL;
	}

private:
	uint64_t m_frameID;
	int num_channels_;
	cv::Mat mean_;
	Frame* m_pRGBA;
	Frame* m_pRGBAf;
	double m_confidence_threshold;

	detectNet* m_pDN;
	int m_nBox;
	int m_nBoxMax;
	uint32_t m_nClass;

	float* m_bbCPU;
	float* m_bbCUDA;
	float* m_confCPU;
	float* m_confCUDA;

	string m_className;

public:
	_Universe* m_pUniverse;
	_Stream* m_pStream;

};

}

#endif
#endif
