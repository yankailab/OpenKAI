/*
 * IO.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "IO.h"
#include "../Script/Kiss.h"

namespace kai
{

IO::IO()
{
	m_type = none;
	m_status = unknown;
}

IO::~IO()
{
}

bool IO::init(void* pKiss)
{
	CHECK_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool IO::open(void)
{
	return false;
}

bool IO::isOpen(void)
{
	return (m_status == opening);
}

void IO::close(void)
{
}

int IO::read(uint8_t* pBuf, int nByte)
{
	return -1;
}

bool IO::write(uint8_t* pBuf, int nByte)
{
	return false;
}

bool IO::writeLine(uint8_t* pBuf, int nByte)
{
	return false;
}

IO_TYPE IO::type(void)
{
	return m_type;
}


} /* namespace kai */
