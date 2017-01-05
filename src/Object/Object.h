/*
 * Object.h
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#ifndef SRC_OBJECT_OBJECT_H_
#define SRC_OBJECT_OBJECT_H_

#include "../Base/common.h"


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
	uint64_t	m_frameID;
};

class Object
{
public:
	Object(int nObj, uint64_t lTime);
	virtual ~Object();

	int size(void);
	bool add(OBJECT* pNewObj);
	OBJECT* get(int i, uint64_t frameID);
	OBJECT* getByClass(int iClass);

private:
	OBJECT* m_pObj;
	int m_nObj;
	int m_iObj;
	uint64_t m_lifetime;

};

}

#endif
