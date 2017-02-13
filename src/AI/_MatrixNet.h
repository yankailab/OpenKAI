/*
 * _MatrixNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef AI__MatrixNet_H_
#define AI__MatrixNet_H_

#include "../Base/common.h"
#include "_AIbase.h"
#include "_ImageNet.h"

namespace kai
{

class _MatrixNet: public _AIbase
{
public:
	_MatrixNet();
	~_MatrixNet();

	bool init(void* pKiss);
	bool link(void);
	bool draw(void);

	bool bFound(int iClass);
	//	OBJECT** getObjItr(void);

public:
	_ImageNet* m_pIN;

	double m_minProb;
	double m_w;
	double m_h;
	double m_dW;
	double m_dH;
	vDouble4 m_area;
	OBJECT** m_pObj;
	int		m_nObj;


};

}

#endif
