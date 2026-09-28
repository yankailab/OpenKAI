/*
 * _ColorConvert.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__ColorConvert_H_
#define OpenKAI_src_Vision__ColorConvert_H_

#include "../_RGBbase.h"

namespace kai
{

	class _ColorConvert : public _RGBbase
	{
	public:
		_ColorConvert();
		virtual ~_ColorConvert();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ColorConvert *)This)->update();
			return NULL;
		}

	protected:
		RGBframe *m_pRGBin = nullptr;
		int m_code = COLOR_RGB2GRAY;
	};

}
#endif
