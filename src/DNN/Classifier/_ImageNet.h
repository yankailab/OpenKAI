/*
 * _ImageNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__ImageNet_H_
#define OPENKAI_SRC_DNN__ImageNet_H_

#include "../../Base/common.h"
#include "../../Detector/_DetectorBase.h"

namespace kai
{

class _ImageNet: public _DetectorBase
{
public:
	_ImageNet();
	~_ImageNet();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	int classify(Frame* pImg, string* pName);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ImageNet*) This)->update();
		return NULL;
	}

public:
#ifdef USE_TENSORRT
	imageNet* m_pIN;
#endif
	Frame* m_pRGBA;

	int m_nBatch;
	string m_blobIn;
	string m_blobOut;
	int	   m_maxPix;

};

}

#endif
