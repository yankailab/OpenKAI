/*
 * _DetectNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_TensorRT__DetectNet_H_
#define OpenKAI_src_DNN_TensorRT__DetectNet_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_JETSON_INFERENCE
//#include <cuda_runtime.h>
//#include "cudaMappedMemory.h"
//#include "cudaNormalize.h"
#include "detectNet.h"
#include "../../Vision/_VisionBase.h"
#include "../../Detector/_DetectorBase.h"

namespace kai
{

enum DETECTNET_TYPE
{
	detectNet_uff,
	detectNet_cafffe,
};

class _DetectNet: public _DetectorBase
{
public:
	_DetectNet();
	~_DetectNet();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	bool draw(void);

private:
	bool open(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DetectNet*) This)->update();
		return NULL;
	}

public:
	detectNet* m_pDN;
	int m_nBox;
	uint32_t m_nClass;
	DETECTNET_TYPE m_type;
	float m_thr;
	string m_layerIn;
	string m_layerOut;
	string m_layerNboxOut;
	vInt3 m_vDimIn;
	int m_nMaxBatch;
	int m_precision;
	int m_device;
	bool m_bAllowGPUfallback;

	Frame* m_pRGBA;
	Frame* m_pRGBAf;
	bool	m_bSwapRB;
	vFloat3 m_vMean;

};

}
#endif
#endif
#endif
