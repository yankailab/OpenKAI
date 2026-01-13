/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_ObjectArray.h"

namespace kai
{

	_ObjectArray::_ObjectArray()
	{
		m_pO = nullptr;
		m_nO = 0;
		m_nBuf = 128;
	}

	_ObjectArray::~_ObjectArray()
	{
		DEL(m_pO);
	}

	bool _ObjectArray::init(const json& j)
	{
		IF_F(!this->_ModuleBase::init(j));

		m_nBuf = j.value("nBuf", "");
		IF__(m_nBuf <= 0, OK_ERR_INVALID_VALUE);

		m_pO = new _Object[m_nBuf];
		NULL__(m_pO);

		return true;
	}

	int _ObjectArray::init(int n)
	{
		if(n > 0)
			m_nBuf = n;

		m_pO = new _Object[m_nBuf];
		NULL__(m_pO);

		return true;
	}

	bool _ObjectArray::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _ObjectArray::update(void)
	{
		while (m_pT->bAlive())
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
