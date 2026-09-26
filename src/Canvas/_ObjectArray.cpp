/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_ObjectArray.h"

namespace kai
{

	_ObjectArray::_ObjectArray()
	{
	}

	_ObjectArray::~_ObjectArray()
	{
		DEL(m_pO);
	}

	bool _ObjectArray::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "nBuf", m_nBuf);
		IF_F(m_nBuf <= 0);

		m_pO = new _Object[m_nBuf];
		NULL_F(m_pO);

		return true;
	}

	int _ObjectArray::init(int n)
	{
		if (n > 0)
			m_nBuf = n;

		m_pO = new _Object[m_nBuf];
		NULL_F(m_pO);

		return true;
	}

	bool _ObjectArray::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _ObjectArray::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}

	void _ObjectArray::clear(void)
	{
		m_nO = 0;
	}

	_Object *_ObjectArray::add(_Object &o)
	{
		IF__(m_nO >= m_nBuf, nullptr);

		m_pO[m_nO++] = o;
		return &m_pO[m_nO - 1];
	}

	_Object *_ObjectArray::get(int i)
	{
		IF__(i >= m_nO, nullptr);
		IF__(i < 0, nullptr);
		return &m_pO[i];
	}

	int _ObjectArray::size(void)
	{
		return m_nO;
	}

}
