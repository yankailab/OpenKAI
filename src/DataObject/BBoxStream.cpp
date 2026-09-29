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
        json &j = *m_pJ;

        jKv(j, "nBuf", m_nBuf);
        jKv<float>(j, "vContainerDim", m_vContainerDim);

        return true;
    }

    bool BBoxStream::saveConfig(bool bExport)
    {
        IF_F(!this->DataObjBase::saveConfig(false));

        json &j = *m_pJ;
        j["nBuf"] = m_nBuf;
        j["vContainerDim"] = {m_vContainerDim[0], m_vContainerDim[1], m_vContainerDim[2]};

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

	void BBoxStream::add(const vector<BBOX_OBJ>& vSrc, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);
		const size_t nBuf = m_nBuf > 0 ? static_cast<size_t>(m_nBuf) : 0;

		if (vSrc.size() >= nBuf)
		{
			m_vObj.assign(vSrc.end() - nBuf, vSrc.end());
		}
		else
		{
			const size_t nRetain = nBuf - vSrc.size();
			if (m_vObj.size() > nRetain)
				m_vObj.erase(m_vObj.begin(), m_vObj.end() - nRetain);

			m_vObj.insert(m_vObj.end(), vSrc.begin(), vSrc.end());
		}

		updateTstamp(tStamp);
	}

	uint64_t BBoxStream::get(vector<BBOX_OBJ>& vDest)
	{
		std::shared_lock lock(m_sMutex);
		vDest = m_vObj;
		return getTstamp();
	}

}
