/*
 * _Crop.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Crop_H_
#define OpenKAI_src_Vision__Crop_H_

#include "../_RGBbase.h"

namespace kai
{

	class _Crop : public _RGBbase
	{
	public:
		_Crop();
		virtual ~_Crop();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Crop *)This)->update();
			return NULL;
		}

	protected:
		RGBframe *m_pRGBin = nullptr;
		Vector4i m_vRoi = Vector4i::Zero();
	};

}
#endif
