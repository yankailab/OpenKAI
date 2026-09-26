/*
 * _NavBase.cpp
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#include "_NavBase.h"

namespace kai
{

	_NavBase::_NavBase()
	{
	}

	_NavBase::~_NavBase()
	{
	}

	bool _NavBase::loadConfig(void)
	{
		IF_F(!this->_ReferenceFrame::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "tConfidenceTimeoutNs", m_tConfidenceTimeoutNs);

		return true;
	}

	bool _NavBase::saveConfig(bool bExport)
	{
		IF_F(!_ReferenceFrame::saveConfig(false));

		json &j = *m_pJ;
		j["tConfidenceTimeoutNs"] = m_tConfidenceTimeoutNs;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	float _NavBase::confidence(void)
	{
		std::lock_guard<std::mutex> lock(m_mtxConfidence);
		if (m_tConfidenceTimeoutNs &&
			getTns() - m_tConfidenceUpdatedNs >= m_tConfidenceTimeoutNs)
		{
			return 0.0f;
		}
		return m_confidence;
	}

	void _NavBase::setConfidence(float confidence)
	{
		std::lock_guard<std::mutex> lock(m_mtxConfidence);
		m_confidence = std::isfinite(confidence) ? std::clamp(confidence, 0.0f, 100.0f) : 0.0f;
		m_tConfidenceUpdatedNs = getTns();
	}

	void _NavBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ReferenceFrame::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("confidence=" + f2str(confidence()));
	}

}
