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

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Depth2Gray *)This)->update();
			return NULL;
		}

	protected:
		_RGBDbase *m_pV;
	};

}
#endif
