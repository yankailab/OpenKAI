/*
 * _InfiRay.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__InfiRay_H_
#define OpenKAI_src_Vision__InfiRay_H_

#include "_VisionBase.h"

namespace kai
{

	class _InfiRay : public _VisionBase
	{
	public:
		_InfiRay();
		virtual ~_InfiRay();

		int init(void *pKiss);
		int start(void);
		bool open(void);
		void close(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_InfiRay *)This)->update();
			return NULL;
		}

	public:
		int m_deviceID;
		VideoCapture m_InfiRay;
		int m_nInitRead;
		bool m_bResetCam;
	};

}
#endif
