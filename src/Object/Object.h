/*
 * Object.h
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#ifndef SRC_OBJECT_OBJECT_H_
#define SRC_OBJECT_OBJECT_H_

#include "../Base/common.h"

#define N_OBJ 128

namespace kai
{

struct OBJECT
{
	vInt4 		m_bbox;
	vInt2		m_camSize;
	double 		m_dist;
	double		m_prob;
	int			m_iClass;
	string		m_name;
	uint8_t		m_safety;
	vector<Point> m_contour;
};

class Object
{
public:
	Object();
	virtual ~Object();

	void reset(void);
	bool add(OBJECT* pNewObj);
	OBJECT* get(int i);
	OBJECT* getByClass(int iClass);
	int size(void);

public:
	OBJECT m_pObj[N_OBJ];
	int m_nObj;

};

}

#endif
