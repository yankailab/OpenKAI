/*
 * _NavBase.h
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__NavBase_H_
#define OpenKAI_src_Navigation__NavBase_H_

#include "../Universe/_ReferenceFrame.h"

namespace kai
{

	class _NavBase : public _ReferenceFrame
	{
	public:
		_NavBase();
		virtual ~_NavBase();

		virtual bool init(const json& j);
		virtual void console(void *pConsole);

		virtual bool bOpened(void);

		virtual void reset(void);
		virtual float confidence(void);

	protected:
		virtual void resetAll(void);

	protected:
		bool m_bOpened = false;
		float m_confidence = 0.0;
	};

}
#endif
