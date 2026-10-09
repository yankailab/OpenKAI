/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "BBoxStream.h"

namespace kai
{

	BBoxStream::BBoxStream()
	{
		clear(m_nBuf);
	}

	BBoxStream::~BBoxStream()
	{
	}

	bool BBoxStream::loadConfig(void)
	{
		IF_F(!this->DataObjStream::loadConfig());
		std::unique_lock lock(m_sMutex);
		json &j = *m_pJ;

		jKv(j, "nBuf", m_nBuf);
		jKv<float>(j, "vContainerDim", m_vContainerDim);

		lock.unlock();
		return clear(m_nBuf);
	}

	bool BBoxStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjStream::saveConfig(false));
		{
			std::shared_lock lock(m_sMutex);
			json &j = *m_pJ;
			j["nBuf"] = m_nBuf;
			j["vContainerDim"] = {m_vContainerDim[0], m_vContainerDim[1], m_vContainerDim[2]};
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool BBoxStream::clear(size_t nBuf)
	{
		std::unique_lock lock(m_sMutex);

		IF_F((nBuf > std::numeric_limits<int>::max()));
		if (nBuf > 0)
		{
			m_nBuf = nBuf;
		}

		IF_F(m_nBuf <= 0);

		m_vElement.resize(m_nBuf);
		for (BBOX_OBJ &b : m_vElement)
		{
			b.clear();
		}

		m_iBset = 0;
		// Keep the timestamp watermark so existing readers survive a clear.
		updateTstamp();

		return true;
	}

	void BBoxStream::setContainerDim(const Vector3f &vDim)
	{
		std::unique_lock lock(m_sMutex);
		m_vContainerDim = vDim;
	}

	Vector3f BBoxStream::getContainerDim(void)
	{
		std::shared_lock lock(m_sMutex);
		return m_vContainerDim;
	}

}
