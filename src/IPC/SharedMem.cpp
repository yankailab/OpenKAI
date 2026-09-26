/*
 * SharedMem.cpp
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#include "SharedMem.h"
#include "../UI/_Console.h"

namespace kai
{

	SharedMem::SharedMem()
	{
	}

	SharedMem::~SharedMem()
	{
		close();
	}

	bool SharedMem::loadConfig(void)
	{
		IF_F(!this->BASE::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "shmName", m_shmName);
		jKv(j, "nB", m_nB);
		jKv(j, "bWriter", m_bWriter);

		IF_F(!open());

		return true;
	}

	bool SharedMem::saveConfig(bool bExport)
	{
		if (!BASE::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["shmName"] = m_shmName;
		j["nB"] = m_nB;
		j["bWriter"] = m_bWriter;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool SharedMem::link(void)
	{
		IF_F(!this->BASE::link());

		return true;
	}

	bool SharedMem::open(void)
	{
		IF__(m_bOpened, true);

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

		m_bOpened = true;
		return true;
	}

	bool SharedMem::bOpen(void)
	{
		return m_bOpened;
	}

	void SharedMem::close(void)
	{
		IF_(!m_bOpened);

		m_bOpened = false;
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
		IF__(!m_bOpened, nullptr);

		return m_pB;
	}

	bool SharedMem::bWriter(void)
	{
		return m_bWriter;
	}

}
