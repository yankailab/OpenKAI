/*
 * _Crop.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Crop_H_
#define OpenKAI_src_Vision__Crop_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Crop : public _VisionBase
	{
	public:
		_Crop();
		virtual ~_Crop();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
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
		_VisionBase *m_pV;
		vInt4 m_vRoi;
	};

}
#endif
