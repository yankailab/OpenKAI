/*
 * SharedMem.cpp
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#include "SharedMem.h"
#include "../Module/Kiss.h"
#include "../UI/_Console.h"

namespace kai
{

	SharedMem::SharedMem()
	{
		m_shmName = "";
		m_nB = 0;
		m_fd = 0;
		m_pB = 0;
		m_bWriter = true;
		m_bOpen = false;
	}

	SharedMem::~SharedMem()
	{
		close();
	}

	int SharedMem::init(void *pKiss)
	{
		CHECK_(this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("shmName", &m_shmName);
		pK->v("nB", &m_nB);
		pK->v("bWriter", &m_bWriter);

		IF__(!open(), OK_ERR_NOT_READY);

		return OK_OK;
	}

	int SharedMem::link(void)
	{
		CHECK_(this->BASE::link());

		return OK_OK;
	}

	bool SharedMem::open(void)
	{
		IF__(m_bOpen, true);

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

	bool SharedMem::bOpen(void)
	{
		return m_bOpen;
	}

	void SharedMem::close(void)
	{
		IF_(!m_bOpen);

		m_bOpen = false;
		m_pB = NULL;

		if (!m_bWriter)
		{
//			shm_unlink(m_shmName.c_str());
		}
	}

	int SharedMem::nB(void)
	{
		return m_nB;
	}

	void *SharedMem::p(void)
	{
		IF__(!m_bOpen, nullptr);

		return m_pB;
	}

	bool SharedMem::bWriter(void)
	{
		return m_bWriter;
	}

}
