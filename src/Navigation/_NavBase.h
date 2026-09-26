/*
 * _NavBase.h
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__NavBase_H_
#define OpenKAI_src_Navigation__NavBase_H_

#include "../Universe/_ReferenceFrame.h"
#include <mutex>

namespace kai
{

	class _NavBase : public _ReferenceFrame
	{
	public:
		_NavBase();
		virtual ~_NavBase();

		virtual bool loadConfig(void) override;
		virtual void console(void *pConsole);

		virtual float confidence(void);
		// Navigation confidence in percent; zero means unavailable or stale.
		virtual void setConfidence(float confidence);

		//TODO: add global positioning, etc.

	protected:
		float m_confidence = 0.0;
		uint64_t m_tConfidenceTimeoutNs = 0; // zero disables expiry
		uint64_t m_tConfidenceUpdatedNs = 0;
		std::mutex m_mtxConfidence;
	};

}
#endif
