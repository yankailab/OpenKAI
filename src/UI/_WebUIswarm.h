/*
 * _WebUIswarm.h
 *
 *  Created on: July 10, 2023
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WebUIswarm_H_
#define OpenKAI_src_UI_WebUIswarm_H_

#include "_WebUIbase.h"

namespace kai
{
	class _WebUIswarm : public _WebUIbase
	{
	public:
		_WebUIswarm();
		virtual ~_WebUIswarm();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);

	protected:
		void newWindow(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_WebUIswarm *)This)->update();
			return NULL;
		}

	protected:

	};
}
#endif
