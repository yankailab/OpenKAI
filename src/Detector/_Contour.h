/*
 * _Contour.h
 *
 *  Created on: Jan 18, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Contour_H_
#define OpenKAI_src_Detector__Contour_H_

#include "_DetectorBase.h"

namespace kai
{

	class _Contour : public _DetectorBase
	{
	public:
		_Contour();
		virtual ~_Contour();

		virtual int init(void *pKiss);
		virtual int start(void);
		virtual int check(void);
		virtual void draw(void *pFrame);

	private:
		void detect(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Contour *)This)->update();
			return NULL;
		}

	protected:
		int m_mode;
		int m_method;
	};

}
#endif
