/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"

namespace kai
{

BASE::BASE()
{
	m_pKiss = NULL;
}

BASE::~BASE()
{
	// TODO Auto-generated destructor stub
}

bool BASE::init(Kiss* pKiss)
{
	if (!pKiss)
		return false;

	string name;
	if(!pKiss->v("name",&name))return false;
	if (name.empty())return false;

	m_pKiss = pKiss;
	return true;
}

string* BASE::getName(void)
{
	if(!m_pKiss)return NULL;
	return &m_pKiss->m_name;
}

string* BASE::getClass(void)
{
	if(!m_pKiss)return NULL;
	return &m_pKiss->m_class;
}


}
