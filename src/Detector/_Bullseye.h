/*
 * _Bullseye.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Bullseye_H_
#define OpenKAI_src_Detector__Bullseye_H_

#include "../Detector/_DetectorBase.h"

namespace kai
{

	class _Bullseye : public _DetectorBase
	{
	public:
		_Bullseye();
		virtual ~_Bullseye();

		bool init(void *pKiss);
		bool start(void);
		int check(void);

	private:
		void detect(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Bullseye *)This)->update();
			return NULL;
		}

	protected:
		double m_abs;
		double m_scale1;
		double m_scale2;
		double m_thr1;
		double m_thr2;
	};

}
#endif
