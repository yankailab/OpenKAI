/*
 * RGBbase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__RGBbase_H_
#define OpenKAI_src_Vision__RGBbase_H_

#include "../Base/cv.h"

#include "../DataObject/RGBframe.h"

#include "../UI/_Console.h"
#include "../Protocol/_JSONbase.h"


namespace kai
{
	class _RGBbase : public _ModuleBase
	{
	public:
		_RGBbase();
		virtual ~_RGBbase();

		virtual bool loadConfig(void) override;
		virtual bool saveConfig(bool bExport) override;

		virtual bool link(InstanceMgr *pM) override;
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual bool open(void);
		virtual bool bOpened(void);
		virtual void close(void);

	protected:
		// output
		RGBframe *m_pRGB = nullptr;

		// config
		string m_devURI = "";
		bool m_bRGB = true;
		int m_devFPS = 30; // device native FPS
		Vector2i m_vSizeRGB = Vector2i(1280, 720);

		// state
		bool m_bOpened = false;

	};

}
#endif
