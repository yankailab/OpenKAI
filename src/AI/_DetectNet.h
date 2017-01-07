/*
 * _DetectNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef AI__DetectNet_H_
#define AI__DetectNet_H_

#include "../Base/common.h"
#include "_AIbase.h"
#include "../Stream/_StreamBase.h"

#ifdef USE_TENSORRT

namespace kai
{

class _DetectNet: public _AIbase
{
public:
	_DetectNet();
	~_DetectNet();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
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
	int m_nBoxMax;
	uint32_t m_nClass;

	int num_channels_;
	cv::Mat mean_;
	Frame* m_pRGBA;
	Frame* m_pRGBAf;
	double m_minCofidence;

	float* m_bbCPU;
	float* m_bbCUDA;
	float* m_confCPU;
	float* m_confCUDA;

	string m_className;

public:

};

}

#endif
#endif
