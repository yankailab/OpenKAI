/*
 * _Resize.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Resize_H_
#define OpenKAI_src_Vision__Resize_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Resize : public _VisionBase
	{
	public:
		_Resize();
		virtual ~_Resize();

		bool init(void *pKiss);
		bool start(void);
		bool open(void);
		void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Resize *)This)->update();
			return NULL;
		}

	public:
		_VisionBase *m_pV;
	};

}
#endif
