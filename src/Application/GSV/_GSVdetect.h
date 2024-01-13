/*
 * _GSVdetect.h
 *
 *  Created on: Jan 14, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_App_GSV__GSVdetect_H_
#define OpenKAI_src_App_GSV__GSVdetect_H_

#include "../../Detector/_DetectorBase.h"

namespace kai
{

	class _GSVdetect : public _DetectorBase
	{
	public:
		_GSVdetect();
		virtual ~_GSVdetect();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void draw(void *pFrame);

	protected:
		void detect(void);
        virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GSVdetect *)This)->update();
			return NULL;
		}

	protected:
	};

}
#endif
