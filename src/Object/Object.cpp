/*
 * _Universe.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#include "Object.h"

namespace kai
{

Object::Object()
{
	m_nObj = 0;
}

Object::~Object()
{
}

void Object::reset(void)
{
	m_nObj = 0;
}

bool Object::add(OBJECT* pNewObj)
{
	NULL_F(pNewObj);
	CHECK_F(m_nObj >= N_OBJ);
	m_pObj[m_nObj++] = *pNewObj;
	return true;
}

OBJECT* Object::get(int i)
{
	CHECK_N(i>=m_nObj);
	return &m_pObj[i];
}

OBJECT* Object::getByClass(int iClass)
{
	int i;
	OBJECT* pObj;

	for (i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_iClass == iClass)
			return pObj;
	}

	return NULL;
}

int Object::size(void)
{
	return m_nObj;
}

}
