/*
 * _DepthShow.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__RSdepth_H_
#define OpenKAI_src_Vision__RSdepth_H_

#include "../_DepthVisionBase.h"

namespace kai
{

	class _DepthShow : public _VisionBase
	{
	public:
		_DepthShow();
		virtual ~_DepthShow();

		bool init(void *pKiss);
		bool start(void);
		bool open(void);
		void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_DepthShow *)This)->update();
			return NULL;
		}

	public:
		_DepthVisionBase *m_pV;
	};

}
#endif
