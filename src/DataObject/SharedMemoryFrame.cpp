/*
 * SharedMemoryFrame.cpp
 *
 *  Created on: Sept 20, 2022
 *      Author: yankai
 */

#include "SharedMemoryFrame.h"

namespace kai
{

	SharedMemoryFrame::SharedMemoryFrame()
	{
	}

	SharedMemoryFrame::~SharedMemoryFrame()
	{
		close();
	}

	bool SharedMemoryFrame::loadConfig(void)
	{
		IF_F(!this->BASE::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "shmName", m_shmName);
		jKv(j, "nB", m_nB);
		jKv(j, "bWriter", m_bWriter);

		IF_F(!open());

		return true;
	}

	bool SharedMemoryFrame::saveConfig(bool bExport)
	{
		IF_F(!BASE::saveConfig(false));

		json &j = *m_pJ;
		j["shmName"] = m_shmName;
		j["nB"] = m_nB;
		j["bWriter"] = m_bWriter;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool SharedMemoryFrame::open(void)
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

	bool SharedMemoryFrame::bOpen(void)
	{
		return m_bOpened;
	}

	void SharedMemoryFrame::close(void)
	{
		IF_(!m_bOpened);

		m_bOpened = false;
		m_pB = NULL;

		if (!m_bWriter)
		{
			//			shm_unlink(m_shmName.c_str());
		}
	}

	int SharedMemoryFrame::nB(void)
	{
		return m_nB;
	}

	void *SharedMemoryFrame::p(void)
	{
		IF__(!m_bOpened, nullptr);

		return m_pB;
	}

	bool SharedMemoryFrame::bWriter(void)
	{
		return m_bWriter;
	}

}
