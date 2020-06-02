/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_ObjectArray.h"

namespace kai
{

_ObjectArray::_ObjectArray()
{
	m_pO = NULL;
	m_nO = 0;
	m_nBuf = 16;
}

_ObjectArray::~_ObjectArray()
{
	DEL(m_pO);
}

bool _ObjectArray::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nBuf", &m_nBuf);
	IF_F(m_nBuf <= 0);

	m_pO = new _Object[m_nBuf];

	return true;
}

bool _ObjectArray::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ObjectArray::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->autoFPSto();
	}
}

void _ObjectArray::clear(void)
{
	m_nO = 0;
}

_Object* _ObjectArray::add(_Object& o)
{
	IF_N(m_nO >= m_nBuf);

	m_pO[m_nO++] = o;
	return &m_pO[m_nO-1];
}

_Object* _ObjectArray::get(int i)
{
	IF_N(i >= m_nO);
	IF_N(i < 0);
	return &m_pO[i];
}

int _ObjectArray::size(void)
{
	return m_nO;
}

void _ObjectArray::draw(void)
{
	this->_ThreadBase::draw();
}

}
