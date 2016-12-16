/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"

namespace kai
{

BASE::BASE()
{
	m_pKiss = NULL;
	m_pWindow = NULL;
}

BASE::~BASE()
{
}

bool BASE::init(void* pKiss)
{
	NULL_F(pKiss);
	Kiss* pK = (Kiss*)pKiss;

	string name="";
	F_FATAL_F(pK->v("name",&name));
	CHECK_F(name.empty());

	m_pKiss = pKiss;
	return true;
}

bool BASE::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("Window",&iName));
	m_pWindow = (Window*)(pK->root()->getChildInstByName(&iName));

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

bool BASE::draw(void)
{
	NULL_F(m_pWindow);
	NULL_F(((Window*)m_pWindow)->getFrame());
	return true;
}


}
