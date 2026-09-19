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

	bool _NavBase::init(const json &j)
	{
		IF_F(!this->_ReferenceFrame::init(j));

		jKv(j, "tConfidenceTimeoutUs", m_tConfidenceTimeoutUs);

		return true;
	}

	float _NavBase::confidence(void)
	{
		std::lock_guard<std::mutex> lock(m_mtxConfidence);
		if (m_tConfidenceTimeoutUs &&
			getTbootUs() - m_tConfidenceUpdatedUs >= m_tConfidenceTimeoutUs)
			return 0.0f;
		return m_confidence;
	}

	void _NavBase::setConfidence(float confidence)
	{
		std::lock_guard<std::mutex> lock(m_mtxConfidence);
		m_confidence = std::isfinite(confidence) ? std::clamp(confidence, 0.0f, 100.0f) : 0.0f;
		m_tConfidenceUpdatedUs = getTbootUs();
	}

	void _NavBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ReferenceFrame::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("confidence=" + f2str(confidence()));
	}

}
