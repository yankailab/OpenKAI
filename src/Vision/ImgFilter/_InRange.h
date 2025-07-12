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

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_InRange *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		vInt3 m_vL;
		vInt3 m_vH;
	};

}
#endif
