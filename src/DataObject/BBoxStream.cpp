/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "BBoxStream.h"

namespace kai
{

	BBoxStream::BBoxStream()
	{
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

		return true;
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

	void BBoxStream::add(const vector<BBOX_OBJ> &vSrc, uint64_t tStamp)
	{
		IF_(vSrc.empty());

		std::unique_lock lock(m_sMutex);
		const uint32_t nAdd = std::min(m_nBuf, (uint32_t)vSrc.size());

		if (vSrc.size() >= m_nBuf)
		{
			m_vObj.assign(vSrc.end() - nAdd, vSrc.end());
		}
		else
		{
			const size_t nRetain = m_nBuf - nAdd;
			if (m_vObj.size() > nRetain)
			{
				m_vObj.erase(m_vObj.begin(), m_vObj.end() - nRetain);
			}

			m_vObj.insert(m_vObj.end(), vSrc.begin(), vSrc.end());
		}

		// per element tStamp is given by the producer so we don't touch it here

		updateTstamp(tStamp);
	}

	uint64_t BBoxStream::get(vector<BBOX_OBJ> &vDest)
	{
		std::shared_lock lock(m_sMutex);
		vDest = m_vObj;
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
