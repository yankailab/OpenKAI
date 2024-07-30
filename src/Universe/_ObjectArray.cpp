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
		m_nBuf = 16;
	}

	_ObjectArray::~_ObjectArray()
	{
		DEL(m_pO);
	}

	int _ObjectArray::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nBuf", &m_nBuf);
		IF__(m_nBuf <= 0, OK_ERR_INVALID_VALUE);

		m_pO = new _Object[m_nBuf];
		NULL__(m_pO, OK_ERR_ALLOCATION);

		return OK_OK;
	}

	int _ObjectArray::init(int n)
	{
		if(n > 0)
			m_nBuf = n;

		m_pO = new _Object[m_nBuf];
		NULL__(m_pO, OK_ERR_ALLOCATION);

		return OK_OK;
	}

	int _ObjectArray::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _ObjectArray::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			m_pT->autoFPSto();
		}
	}

	void _ObjectArray::clear(void)
	{
		m_nO = 0;
	}

	_Object *_ObjectArray::add(_Object &o)
	{
		IF_N(m_nO >= m_nBuf);

		m_pO[m_nO++] = o;
		return &m_pO[m_nO - 1];
	}

	_Object *_ObjectArray::get(int i)
	{
		IF_N(i >= m_nO);
		IF_N(i < 0);
		return &m_pO[i];
	}

	int _ObjectArray::size(void)
	{
		return m_nO;
	}

}
