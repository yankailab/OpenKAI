/*
 * UGLIDcellStream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "UGLIDcellStream.h"

namespace kai
{

	UGLIDcellStream::UGLIDcellStream()
	{
		clear(m_nBuf);
	}

	UGLIDcellStream::~UGLIDcellStream()
	{
	}

	bool UGLIDcellStream::loadConfig(void)
	{
		IF_F(!this->DataObjStream::loadConfig());
		std::unique_lock lock(m_sMutex);
		json &j = *m_pJ;

		jKv(j, "nBuf", m_nBuf);

		lock.unlock();
		return clear(m_nBuf);
	}

	bool UGLIDcellStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjStream::saveConfig(false));
		{
			std::shared_lock lock(m_sMutex);
			json &j = *m_pJ;
			j["nBuf"] = m_nBuf;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool UGLIDcellStream::clear(size_t nBuf)
	{
		std::unique_lock lock(m_sMutex);

		IF_F((nBuf > std::numeric_limits<int>::max()));
		if (nBuf > 0)
		{
			m_nBuf = nBuf;
		}

		IF_F(m_nBuf <= 0);

		m_vElement.resize(m_nBuf);
		for (UGLID_CELL_T &c : m_vElement)
		{
			c.clear();
		}

		m_iBset = 0;
		updateTstamp();

		return true;
	}

	void UGLIDcellStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjStream::console(pConsole);
	}

}
