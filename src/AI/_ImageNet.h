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
#include "../Stream/_StreamBase.h"

namespace kai
{

class _ImageNet: public _AIbase
{
public:
	_ImageNet();
	~_ImageNet();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	int size(void);
	bool add(OBJECT* pNewObj);
	OBJECT* get(int i, int64_t frameID);
	OBJECT* getByClass(int iClass);

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
	_StreamBase* m_pStream;
	Frame* m_pRGBA;

	int m_nBatch;
	string m_blobIn;
	string m_blobOut;

	OBJECT* m_pObj;
	int m_nObj;
	int m_iObj;
	int64_t m_obsLifetime;

	double m_distThr;
	double m_detectMinSize;
	double m_extraBBox;
	double m_contourBlend;
	bool m_bDrawContour;

	double m_sizeName;
	double m_sizeDist;
	Scalar m_colName;
	Scalar m_colDist;
	Scalar m_colObs;

};

}

#endif
