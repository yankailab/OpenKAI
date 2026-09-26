/*
 * _Invert.h
 *
 *  Created on: March 14, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Invert_H_
#define OpenKAI_src_Vision__Invert_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Invert : public _VisionBase
	{
	public:
		_Invert();
		virtual ~_Invert();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Invert *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV = nullptr;
	};

}
#endif
