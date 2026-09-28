/*
 * _Rotate.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Rotate_H_
#define OpenKAI_src_Vision__Rotate_H_

#include "../_RGBbase.h"

namespace kai
{

	class _Rotate : public _RGBbase
	{
	public:
		_Rotate();
		virtual ~_Rotate();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Rotate *)This)->update();
			return NULL;
		}

	protected:
		RGBframe *m_pRGBin = nullptr;
		int m_code = 0;
	};

}
#endif
