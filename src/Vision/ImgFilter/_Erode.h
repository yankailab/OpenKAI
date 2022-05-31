/*
 * _Erode.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Erode_H_
#define OpenKAI_src_Vision__Erode_H_

#include "../_VisionBase.h"

namespace kai
{

	struct IMG_ERODE
	{
		int m_nItr;
		int m_kShape;
		int m_kW;
		int m_kH;
		int m_aX;
		int m_aY;
		Mat m_kernel;

		void init(void)
		{
			m_nItr = 1;
			m_kW = 3;
			m_kH = 3;
			m_aX = -1;
			m_aY = -1;
			m_kShape = cv::MORPH_RECT;
		}

		void updateKernel(void)
		{
			m_kernel = getStructuringElement(m_kShape, cv::Size(m_kW, m_kH), cv::Point(m_aX, m_aY));
		}
	};

	class _Erode : public _VisionBase
	{
	public:
		_Erode();
		virtual ~_Erode();

		bool init(void *pKiss);
		bool start(void);
		bool open(void);
		void close(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Erode *)This)->update();
			return NULL;
		}

	public:
		_VisionBase *m_pV;
		Frame m_fIn;
		vector<IMG_ERODE> m_vFilter;
	};

}
#endif
