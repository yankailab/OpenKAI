/*
 * CANframeStream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "CANframeStream.h"

namespace kai
{

	CANframeStream::CANframeStream()
	{
		clear(m_nBuf);
	}

	CANframeStream::~CANframeStream()
	{
	}

	bool CANframeStream::loadConfig(void)
	{
		IF_F(!this->DataObjBase::loadConfig());
		std::unique_lock lock(m_sMutex);
		json &j = *m_pJ;

		jKv(j, "nBuf", m_nBuf);

		lock.unlock();
		return clear(m_nBuf);
	}

	bool CANframeStream::saveConfig(bool bExport)
	{
		IF_F(!this->DataObjBase::saveConfig(false));
		{
			std::shared_lock lock(m_sMutex);
			json &j = *m_pJ;
			j["nBuf"] = m_nBuf;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool CANframeStream::clear(size_t nBuf)
	{
		std::unique_lock lock(m_sMutex);

		IF_F((nBuf > std::numeric_limits<int>::max()));
		if (nBuf > 0)
		{
			m_nBuf = nBuf;
		}

		IF_F(m_nBuf <= 0);

		m_vCframe.resize(m_nBuf);
		for (CAN_FRAME &c : m_vCframe)
		{
			c.clear();
		}

		m_iBset = 0;
		updateTstamp();

		return true;
	}

	void CANframeStream::add(const vector<CAN_FRAME> &vSrc, uint64_t tStamp)
	{
		IF_(vSrc.empty());

		std::unique_lock lock(m_sMutex);

		for (const CAN_FRAME &c : vSrc)
		{
			m_vCframe[m_iBset] = c;
			if (++m_iBset == m_nBuf)
				m_iBset = 0;
		}

		// Per-element timestamps are supplied by the producer.
		updateTstamp(tStamp);
	}

	uint64_t CANframeStream::get(vector<CAN_FRAME> &vDest, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutex);

		vDest.clear();
		const size_t nFrame = m_vCframe.size();
		vDest.reserve(nFrame);

		for (size_t n = 0, iFrame = m_iBset; n < nFrame; ++n)
		{
			const CAN_FRAME &c = m_vCframe[iFrame];
			if (c.m_tStamp > tStampFrom)
				vDest.push_back(c);

			if (++iFrame == nFrame)
				iFrame = 0;
		}

		return getTstamp();
	}

	void CANframeStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
