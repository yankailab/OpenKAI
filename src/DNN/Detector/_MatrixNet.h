/*
 * _MatrixNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__MatrixNet_H_
#define OPENKAI_SRC_DNN__MatrixNet_H_

#include "../../Base/common.h"
#include "../../Detector/_DetectorBase.h"
#include "../Classifier/_ImageNet.h"

namespace kai
{

class _MatrixNet: public _DetectorBase
{
public:
	_MatrixNet();
	~_MatrixNet();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	OBJECT* get(int i);
	bool bFound(int iClass, double minProb);
	void bSetActive(bool bActive);

private:
	vInt4 explore(int x, int y, int iClass);
	void cluster(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MatrixNet*) This)->update();
		return NULL;
	}

public:
	_ImageNet* m_pIN;

	int m_nObj;
	double m_w;
	double m_h;
	double m_dW;
	double m_dH;
	double m_aW;
	double m_aH;
	vInt2 m_size;
	vDouble4 m_area;
	OBJECT** m_ppObj;
	bool m_bActive;

};

}

#endif
