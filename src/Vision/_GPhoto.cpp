/*
 * _GPhoto.cpp
 *
 *  Created on: Feb 20, 2020
 *      Author: yankai
 */

#include "_GPhoto.h"

namespace kai
{

	_GPhoto::_GPhoto()
	{
		m_type = vision_gphoto;

		m_cmdUnmount = "gio mount -s gphoto2";
	}

	_GPhoto::~_GPhoto()
	{
		close();
	}

	bool _GPhoto::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("mount", &m_cmdUnmount);

		return true;
	}

	bool _GPhoto::open(void)
	{
		if (!m_cmdUnmount.empty())
			system(m_cmdUnmount.c_str());

		return true;
	}

	void _GPhoto::close(void)
	{
		this->_VisionBase::close();
	}

	bool _GPhoto::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _GPhoto::update(void)
	{
		while (m_pT->bThread())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			m_pT->autoFPSto();
		}
	}

	bool _GPhoto::shutter(string &fName)
	{
		return true;
	}

}
