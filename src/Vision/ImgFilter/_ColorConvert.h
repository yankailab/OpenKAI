/*
 * _ColorConvert.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__ColorConvert_H_
#define OpenKAI_src_Vision__ColorConvert_H_

#include "../_VisionBase.h"

namespace kai
{

	class _ColorConvert : public _VisionBase
	{
	public:
		_ColorConvert();
		virtual ~_ColorConvert();

		int init(void *pKiss);
		int link(void);
		int check(void);
		int start(void);
		bool open(void);
		void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ColorConvert *)This)->update();
			return NULL;
		}

	public:
		_VisionBase *m_pV;
		int m_code;
	};

}
#endif
