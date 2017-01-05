/*
 * Object.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#include "Object.h"

namespace kai
{

Object::Object(int nObj, uint64_t lTime)
{
	m_lifetime = lTime;
	m_iObj = 0;
	m_nObj = nObj;

	m_pObj = new OBJECT[m_nObj];
	for(int i=0;i<m_nObj;i++)
	{
		m_pObj[i].m_frameID = 0;
	}
}

Object::~Object()
{
}

bool Object::add(OBJECT* pNewObj)
{
	NULL_F(pNewObj);
	m_pObj[m_iObj] = *pNewObj;
	if(++m_iObj>=m_nObj)m_iObj=0;
	return true;
}

int Object::size(void)
{
	return m_nObj;
}

OBJECT* Object::get(int i, uint64_t frameID)
{
	CHECK_N(frameID - m_pObj[i].m_frameID >= m_lifetime);
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

}
