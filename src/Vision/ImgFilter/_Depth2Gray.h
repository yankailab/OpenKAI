/*
 * _Depth2Gray.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Depth2Gray_H_
#define OpenKAI_src_Vision__Depth2Gray_H_

#include "../RGBD/_RGBDbase.h"

namespace kai
{

	class _Depth2Gray : public _VisionBase
	{
	public:
		_Depth2Gray();
		virtual ~_Depth2Gray();

		int init(void *pKiss);
		int start(void);
		bool open(void);
		void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Depth2Gray *)This)->update();
			return NULL;
		}

	public:
		_RGBDbase *m_pV;
	};

}
#endif
