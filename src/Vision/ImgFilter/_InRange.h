/*
 * _InRange.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__InRange_H_
#define OpenKAI_src_Vision__InRange_H_

#include "../_VisionBase.h"

namespace kai
{

	class _InRange : public _VisionBase
	{
	public:
		_InRange();
		virtual ~_InRange();

		bool init(void *pKiss);
		bool link(void);
		bool start(void);
		bool open(void);
		void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_InRange *)This)->update();
			return NULL;
		}

	public:
		_VisionBase *m_pV;
		vFloat2 m_vRange;
	};

}
#endif
