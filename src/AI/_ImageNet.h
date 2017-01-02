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
#include "../Object/Object.h"
#include "../Stream/_StreamBase.h"

#ifdef USE_TENSORRT

namespace kai
{

class _ImageNet: public _AIbase
{
public:
	_ImageNet();
	~_ImageNet();

	bool init(void* pKiss);
	bool link(void);
	bool draw(void);

	bool start(void);

	int addObjClass(string* pName, uint8_t safety);
	OBJECT* addObject(OBJECT* pNewObj);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ImageNet*) This)->update();
		return NULL;
	}

public:
	imageNet* m_pIN;

	int m_nBatch;
	string m_blobIn;
	string m_blobOut;

	Frame* m_pRGBA;

public:

};

}

#endif
#endif
