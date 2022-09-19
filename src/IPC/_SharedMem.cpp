/*
 * _SharedMem.cpp
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#include "_SharedMem.h"

namespace kai
{

	_SharedMem::_SharedMem()
	{
		m_shmName = "";
		m_nB = 0;
		m_fd = 0;
		m_pB = 0;
		m_bWriter = true;
		m_bOpen = false;
	}

	_SharedMem::~_SharedMem()
	{
	}

	bool _SharedMem::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("shmName", &m_shmName);
		pK->v("nB", &m_nB);
		pK->v("bWriter", &m_bWriter);

		return true;
	}

	bool _SharedMem::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		return true;
	}

	bool _SharedMem::open(void)
	{
		IF_T(m_bOpen);

		if (m_bWriter)
		{
			m_fd = shm_open(m_shmName.c_str(), O_CREAT | O_RDWR, 0666);
			IF_F(m_fd == 0);

			ftruncate(m_fd, m_nB);
			m_pB = mmap(0, m_nB, PROT_WRITE, MAP_SHARED, m_fd, 0);
		}
		else
		{
			m_fd = shm_open(m_shmName.c_str(), O_RDONLY, 0666);
			IF_F(m_fd == 0);

			m_pB = mmap(0, m_nB, PROT_READ, MAP_SHARED, m_fd, 0);
		}

		m_bOpen = true;
		return true;
	}

	bool _SharedMem::isOpen(void)
	{
		return m_bOpen;
	}

	void _SharedMem::close(void)
	{
		IF_(!m_bOpen);

		m_bOpen = false;
		m_pB = NULL;

		if (!m_bWriter)
		{
			shm_unlink(m_shmName.c_str());
		}
	}

	void *_SharedMem::p(void)
	{
		IF_N(!m_bOpen);

		return m_pB;
	}

	bool _SharedMem::bWriter(void)
	{
		return m_bWriter;
	}

}
