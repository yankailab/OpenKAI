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
	m_name = "";

}

BASE::~BASE()
{
	// TODO Auto-generated destructor stub
}

bool BASE::init(Config* pConfig, string* pName)
{
	if (!pConfig)
		return false;

	if (!pName)
		return false;

	if (pName->empty())
		return false;

	Config* pC = pConfig->o(*pName);
	if (pC->empty())
		return false;

	m_name = *pName;
	return true;
}

}
