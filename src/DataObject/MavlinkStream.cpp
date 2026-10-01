/*
 * MavlinkStream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavlinkStream.h"

namespace kai
{

	MavlinkStream::MavlinkStream()
	{
	}

	MavlinkStream::~MavlinkStream()
	{
	}

	bool MavlinkStream::loadConfig(void)
	{
		IF_F(!this->DataObjBase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "nMsg", m_nMsg);

		return clear(m_nMsg);
	}

	bool MavlinkStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjBase::saveConfig(false));
		{
			std::shared_lock lock(m_sMutex);
			json &j = *m_pJ;
			j["nMsg"] = m_nMsg;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool MavlinkStream::clear(size_t nMsg)
	{
		std::unique_lock lock(m_sMutex);

		m_vMsg.clear();
		if (nMsg > 0)
		{
			m_vMsg.reserve(nMsg);
		}

		for (MAVLINK_MSG &mm : m_vMsg)
		{
			mm.clear();
		}

		m_iMset = 0;

		return true;
	}

	void MavlinkStream::addMsg(MavMsgBase& msg, uint64_t tStamp)
	{
		std::unique_lock lock(m_sMutex);

		MAVLINK_MSG *pM = &m_vMsg[m_iMset];
		pM->set(msg, tStamp);

		if (m_iMset == m_vMsg.size() - 1)
		{
			m_iMset = 0;
		}
		else
		{
			m_iMset++;
		}
	}

	void MavlinkStream::getMsgs(vector<MAVLINK_MSG> &vMsg, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutex);

		vMsg.clear();
		const size_t nMsg = m_vMsg.size();
		vMsg.reserve(nMsg);

		size_t iMsg = m_iMset;
		for (size_t n = 0; n < nMsg; ++n)
		{
			iMsg = iMsg == 0 ? nMsg - 1 : iMsg - 1;
			const MAVLINK_MSG &m = m_vMsg[iMsg];
			if (m.m_tStamp < tStampFrom)
				break;

			vMsg.push_back(m);
		}
	}

	void MavlinkStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
