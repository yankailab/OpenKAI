/*
 * _OCVwindow.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_OCVwindow_H_
#define OpenKAI_src_UI_OCVwindow_H_

#include "../DataObject/RGBframe.h"
#include "../DataObject/BBoxStream.h"

#include "../Base/_ModuleBase.h"

#include "../Base/cv.h"
#include <opencv2/highgui.hpp>

namespace kai
{
	class _OCVwindow : public _ModuleBase
	{
	public:
		_OCVwindow();
		virtual ~_OCVwindow();

		virtual bool loadConfig(void) override;
		virtual bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual bool check(void);

	protected:
		void updateWindow(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OCVwindow *)This)->update();
			return NULL;
		}

	protected:
		RGBframe *m_pRGBin = nullptr;
		BBoxStream *m_pBBoxIn = nullptr;

		Vector2i m_vSize = Vector2i(1280, 720);
		bool m_bFullScreen = false;

		string m_gstOutput = "";
		VideoWriter m_gst;
	};

}
#endif
