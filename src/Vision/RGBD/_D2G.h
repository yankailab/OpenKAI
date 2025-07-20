/*
 * _D2G.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Depth2Gray_H_
#define OpenKAI_src_Vision__Depth2Gray_H_

#include "../RGBD/_RGBDbase.h"

namespace kai
{

	class _D2G : public _VisionBase
	{
	public:
		_D2G();
		virtual ~_D2G();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_D2G *)This)->update();
			return NULL;
		}

	protected:
		_RGBDbase *m_pV;
	};

}
#endif
