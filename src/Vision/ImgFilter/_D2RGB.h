/*
 * _D2RGB.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__D2RGB_H_
#define OpenKAI_src_Vision__D2RGB_H_

#include "../RGBD/_RGBDbase.h"

namespace kai
{

	class _D2RGB : public _VisionBase
	{
	public:
		_D2RGB();
		virtual ~_D2RGB();

		int init(void *pKiss);
		int link(void);

		Frame *getFrameRGB(void);

	protected:
		_RGBDbase *m_pV;
	};

}
#endif
