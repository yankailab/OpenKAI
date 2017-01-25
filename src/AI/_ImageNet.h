/*
 * _ImageNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef AI__ImageNet_H_
#define AI__ImageNet_H_

#include "../Base/common.h"
#include "_AIbase.h"

namespace kai
{

enum imageNetMode
{
	depth, object, noThread
};

class _ImageNet: public _AIbase
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
	void detectObject(void);
	void detectDepth(void);
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

	imageNetMode m_mode;
	int m_nBatch;
	string m_blobIn;
	string m_blobOut;

	double m_fDist;
	double m_detectMinSize;
	double m_detectMaxSize;
	double m_extraBBox;
	int	   m_maxPix;


};

}

#endif
