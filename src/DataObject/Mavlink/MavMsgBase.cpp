/*
 * MavMsgBase.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "MavMsgBase.h"

namespace kai
{

	void MavCallback::callback(void *pMavMsg)
	{
		NULL_(m_pCbRecv);
		m_pCbRecv(pMavMsg, m_pCbInst);
	}

	MavMsgBase::MavMsgBase()
	{
	}

	MavMsgBase::~MavMsgBase(void)
	{
	}

	uint32_t MavMsgBase::getID(void)
	{
		return m_id;
	}

	void MavMsgBase::decode(const mavlink_message_t &msg)
	{
	}

	bool MavMsgBase::bValid(void)
	{
		return m_tStamp > 0;
	}

	void MavMsgBase::setDesiredInterval(int64_t tIntervalNsec)
	{
		m_tDesiredInterval = tIntervalNsec;
	}

	int64_t MavMsgBase::getDesiredInterval(void)
	{
		return m_tDesiredInterval;
	}

	bool MavMsgBase::bOnTime(void)
	{
		IF__(m_tActualInterval < m_tDesiredInterval + m_tIntervalDelayAllowed, true);

		return false;
	}


	void MavMsgBase::updateTstamp(uint64_t tStamp)
	{
		if (tStamp == 0)
			tStamp = getTns();

		m_tActualInterval = tStamp - m_tStamp;
		m_tStamp = tStamp;
	}

	uint64_t MavMsgBase::getTstamp(void)
	{
		return m_tStamp;
	}


	void MavMsgBase::callbackAll(void)
	{
		// Unregistering waits for active callbacks; callbacks may unregister themselves.
		std::lock_guard<std::recursive_mutex> lock(m_cbMutex);

		const auto callbacks = m_vCbRecv;
		for (MavCallback c : callbacks)
		{
			c.callback(this);
		}
	}

	bool MavMsgBase::addCbRecv(CbMavMsg pCb, void *pInst)
	{
		NULL_F(pCb);
		std::lock_guard<std::recursive_mutex> lock(m_cbMutex);

		for (MavCallback c : m_vCbRecv)
		{
			IF__((c.m_pCbRecv == pCb) && (c.m_pCbInst == pInst), true);
		}

		MavCallback cb;
		cb.m_pCbRecv = pCb;
		cb.m_pCbInst = pInst;
		m_vCbRecv.push_back(cb);

		return true;
	}

	void MavMsgBase::clearCbRecv(CbMavMsg pCb, void *pInst)
	{
		NULL_(pCb);
		std::lock_guard<std::recursive_mutex> lock(m_cbMutex);

		for (auto it = m_vCbRecv.begin(); it != m_vCbRecv.end(); ++it)
		{
			MavCallback *pC = &(*it);
			IF_CONT((pC->m_pCbRecv != pCb) || (pC->m_pCbInst != pInst));

			m_vCbRecv.erase(it);

			return;
		}
	}

	void MavMsgBase::clearAllCbRecv(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_cbMutex);
		m_vCbRecv.clear();
	}

}
