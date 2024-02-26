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
#include "../Vision/Frame.h"
#include "../Utility/utilCV.h"

namespace kai
{
	class _WindowCV : public _UIbase
	{
	public:
		_WindowCV();
		virtual ~_WindowCV();

		bool init(void *pKiss);
		bool start(void);

	protected:
		void updateWindow(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_WindowCV *)This)->update();
			return NULL;
		}

	protected:
		Frame m_F;
		vInt2 m_vSize;

		int m_waitKey;
		bool m_bFullScreen;
	};

}
#endif
