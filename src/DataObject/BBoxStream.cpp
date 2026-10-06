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
		IF_F(!this->DataObjBase::loadConfig());
		std::unique_lock lock(m_sMutex);
		json &j = *m_pJ;

		jKv(j, "nBuf", m_nBuf);
		jKv<float>(j, "vContainerDim", m_vContainerDim);

		lock.unlock();
		return clear(m_nBuf);
	}

	bool BBoxStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjBase::saveConfig(false));
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

		m_vObj.resize(m_nBuf);
		for (BBOX_OBJ &b : m_vObj)
		{
			b.clear();
		}

		m_iBset = 0;
		// Keep the timestamp watermark so existing readers survive a clear.
		updateTstamp();

		return true;
	}

	void BBoxStream::add(const vector<BBOX_OBJ> &vSrc, uint64_t tStamp)
	{
		IF_(vSrc.empty());

		std::unique_lock lock(m_sMutex);

		for (const BBOX_OBJ &b : vSrc)
		{
			m_vObj[m_iBset] = b;
			if (++m_iBset == m_nBuf)
				m_iBset = 0;
		}

		// per element tStamp is given by the producer so we don't touch it here
		updateTstamp(tStamp);
	}

	uint64_t BBoxStream::get(vector<BBOX_OBJ> &vDest, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutex);

		vDest.clear();
		const size_t nObj = m_vObj.size();
		vDest.reserve(nObj);

		for (size_t n = 0, iObj = m_iBset; n < nObj; ++n)
		{
			const BBOX_OBJ &b = m_vObj[iObj];
			if (b.m_tStamp > tStampFrom)
				vDest.push_back(b);

			if (++iObj == nObj)
				iObj = 0;
		}

		return getTstamp();
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
