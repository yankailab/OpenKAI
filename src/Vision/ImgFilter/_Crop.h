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

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual bool open(void);
		virtual void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Crop *)This)->update();
			return NULL;
		}

	public:
		_VisionBase *m_pV;
		vInt4 m_vRoi;
	};

}
#endif
