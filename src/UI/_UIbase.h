/*
 * _UIbase.h
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_UIbase_H_
#define OpenKAI_src_UI_UIbase_H_

#include "../Base/_ModuleBase.h"
#include "../DataStream/RGBframe.h"

namespace kai
{
	class _UIbase : public _ModuleBase
	{
	public:
		_UIbase();
		virtual ~_UIbase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);

	protected:
		static Mat prepareImage(const Mat &image, const Vector2i &size);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_UIbase *)This)->update();
			return NULL;
		}

	protected:
		RGBframe *m_pRGBin = nullptr;

	};
}
#endif
