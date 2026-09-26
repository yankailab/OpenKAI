/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WindowCV_H_
#define OpenKAI_src_UI_WindowCV_H_

#include <opencv2/highgui.hpp>
#include "_UIbase.h"
#include "../Base/cv.h"
#include "../Utility/utilCV.h"

namespace kai
{
	class _WindowCV : public _UIbase
	{
	public:
		_WindowCV();
		virtual ~_WindowCV();

		virtual bool loadConfig(void) override;
		virtual bool start(void);

	protected:
		void updateWindow(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_WindowCV *)This)->update();
			return NULL;
		}

	protected:
		Mat m_M;
		Vector2i m_vSize = Vector2i::Zero();

		bool m_bFullScreen = false;
	};

}
#endif
