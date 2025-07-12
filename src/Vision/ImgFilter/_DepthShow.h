/*
 * _DepthShow.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__RSdepth_H_
#define OpenKAI_src_Vision__RSdepth_H_

#include "../RGBD/_RGBDbase.h"

namespace kai
{

	class _DepthShow : public _VisionBase
	{
	public:
		_DepthShow();
		virtual ~_DepthShow();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_DepthShow *)This)->update();
			return NULL;
		}

	protected:
		_RGBDbase *m_pV;
	};

}
#endif
