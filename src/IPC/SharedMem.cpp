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
		m_fd = 0;
		m_pB = 0;
		m_bOpen = false;
	}

	SharedMem::~SharedMem()
	{
		close();
	}

	bool SharedMem::init(const json &j)
	{
		IF_F(!this->BASE::init(j));

		m_shmName = j.value("shmName", "");
		m_nB = j.value("nB", 0);
		m_bWriter = j.value("bWriter", true);

		IF_F(!open());

		return true;
	}

	bool SharedMem::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->BASE::link(j, pM));

		return true;
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
