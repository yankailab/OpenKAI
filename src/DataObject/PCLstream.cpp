#include "PCLstream.h"
#include <limits>

namespace kai
{
	PCLstream::PCLstream()
	{
		m_vElement.resize(m_nBuf);
	}

	bool PCLstream::loadConfig()
	{
		IF_F(!DataObjStream::loadConfig());
		std::unique_lock lock(m_sMutex);
		size_t nBuf = m_nBuf;
		jKv(*m_pJ, "nBuf", nBuf);
		IF_Le_F(nBuf > std::numeric_limits<int>::max(), "Invalid PCLstream capacity");
		m_vElement.assign(nBuf, GEOMETRY_POINT());
		m_nBuf = nBuf;
		m_iBset = 0;
		// Initial allocation is not a publication. Reloading an existing
		// stream does notify readers that its retained history was cleared.
		if (getTstamp()) updateTstamp();
		return true;
	}

	bool PCLstream::saveConfig(bool bExport)
	{
		IF_F(!DataObjStream::saveConfig(false));
		{
			std::shared_lock lock(m_sMutex);
			(*m_pJ)["nBuf"] = m_nBuf;
		}
		return !bExport || m_pJcfg->saveToFile();
	}

	bool PCLstream::clear(size_t nBuf)
	{
		std::unique_lock lock(m_sMutex);
		IF_F(nBuf > std::numeric_limits<int>::max());
		if (nBuf) m_nBuf = nBuf;
		m_vElement.assign(m_nBuf, GEOMETRY_POINT());
		m_iBset = 0;
		updateTstamp();
		return true;
	}

	uint64_t PCLstream::get(vector<GEOMETRY_POINT> &dest, uint64_t tStampFrom)
	{
		return DataObjStream::get(dest, tStampFrom);
	}
}
