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
	m_bShow = true;
}

BASE::~BASE()
{
	// TODO Auto-generated destructor stub
}

bool BASE::init(void* pKiss)
{
	NULL_F(pKiss);
	Kiss* pK = (Kiss*)pKiss;

	string name;
	CHECK_F(!pK->v("name",&name));
	CHECK_F(name.empty());

	F_INFO(pK->v("bShow", &m_bShow));

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
	NULL_F(pFrame);
	NULL_F(pTextPos);

	return m_bShow;
}


}
