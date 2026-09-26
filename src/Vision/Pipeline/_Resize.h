/*
 * _Resize.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Resize_H_
#define OpenKAI_src_Vision__Resize_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Resize : public _VisionBase
	{
	public:
		_Resize();
		virtual ~_Resize();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Resize *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV = nullptr;
	};

}
#endif
