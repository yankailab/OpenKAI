/*
 * _Rotate.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Rotate_H_
#define OpenKAI_src_Vision__Rotate_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Rotate : public _VisionBase
	{
	public:
		_Rotate();
		virtual ~_Rotate();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Rotate *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		int m_code;
	};

}
#endif
