/*
 * _Mask.h
 *
 *  Created on: July 2, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Mask_H_
#define OpenKAI_src_Vision__Mask_H_

#include "../_RGBbase.h"

namespace kai
{

	class _Mask : public _RGBbase
	{
	public:
		_Mask();
		virtual ~_Mask();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Mask *)This)->update();
			return NULL;
		}

	protected:
		RGBframe *m_pRGBin = nullptr;
		RGBframe *m_pMaskin = nullptr;

	};

}
#endif
