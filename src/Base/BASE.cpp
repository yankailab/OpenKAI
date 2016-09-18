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
	m_pConfig = NULL;
}

BASE::~BASE()
{
	// TODO Auto-generated destructor stub
}

bool BASE::init(Config* pConfig)
{
	if (!pConfig)
		return false;

	string name;
	if(!pConfig->v("name",&name))return false;
	if (name.empty())return false;

	m_pConfig = pConfig;
	return true;
}

string* BASE::getName(void)
{
	if(!m_pConfig)return NULL;
	return &m_pConfig->m_name;
}

string* BASE::getClass(void)
{
	if(!m_pConfig)return NULL;
	return &m_pConfig->m_class;
}


}
