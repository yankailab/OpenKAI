/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_ObjectBase.h"

namespace kai
{

	_ObjectBase::_ObjectBase()
	{
		clear();
	}

	_ObjectBase::~_ObjectBase()
	{
	}

	void _ObjectBase::setType(OBJ_TYPE type)
	{
		m_type = type;
	}

	OBJ_TYPE _ObjectBase::getType(void)
	{
		return m_type;
	}

	void _ObjectBase::setDim(const Vector3f &vD)
	{
		m_vDim = vD;
	}

	void _ObjectBase::addClass(int16_t iClass, int8_t prob)
	{
	}

	int _ObjectBase::getTopClass(void)
	{
	}

	bool _ObjectBase::bClass(int16_t iClass)
	{
	}

	void _ObjectBase::clearClass(void)
	{
	}

}
