/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Script/Kiss.h"

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

bool BASE::init(void* pKiss)
{
	NULL_F(pKiss);

	string name;
	CHECK_F(!((Kiss*)pKiss)->v("name",&name));
	CHECK_F(name.empty());

	m_pKiss = pKiss;
	return true;
}

bool BASE::link(void)
{
	return true;
}

string* BASE::getName(void)
{
	NULL_N(m_pKiss);
	return &((Kiss*)m_pKiss)->m_name;
}

string* BASE::getClass(void)
{
	NULL_N(m_pKiss);
	return &((Kiss*)m_pKiss)->m_class;
}

bool BASE::start(void)
{
	return true;
}

bool BASE::draw(Frame* pFrame, vInt4* pTextPos)
{
	return true;
}


}
