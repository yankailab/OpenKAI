/*
 * _Mask.h
 *
 *  Created on: July 2, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Mask_H_
#define OpenKAI_src_Vision__Mask_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Mask : public _VisionBase
	{
	public:
		_Mask();
		virtual ~_Mask();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Mask *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		_VisionBase *m_pVmask;

		Frame m_fIn;
		Frame m_fMask;
	};

}
#endif
