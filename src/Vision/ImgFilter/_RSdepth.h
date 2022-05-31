/*
 * _RSdepth.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__RSdepth_H_
#define OpenKAI_src_Vision__RSdepth_H_

#include "../_RealSense.h"

namespace kai
{

	class _RSdepth : public _VisionBase
	{
	public:
		_RSdepth();
		virtual ~_RSdepth();

		bool init(void *pKiss);
		bool start(void);
		bool open(void);
		void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_RSdepth *)This)->update();
			return NULL;
		}

	public:
		_RealSense *m_pV;
		Frame m_fIn;
	};

}
#endif
