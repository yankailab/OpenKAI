/*
 * _Morphology.h
 *
 *  Created on: March 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Morphology_H_
#define OpenKAI_src_Vision__Morphology_H_

#include "../_VisionBase.h"

namespace kai
{

	struct IMG_MORPH
	{
		int m_morphOp;
		int m_nItr;
		int m_kShape;
		int m_kW;
		int m_kH;
		int m_aX;
		int m_aY;
		Mat m_kernel;

		void init(void)
		{
			m_morphOp = cv::MORPH_OPEN;
			m_nItr = 1;
			m_kShape = cv::MORPH_RECT;
			m_kW = 3;
			m_kH = 3;
			m_aX = -1;
			m_aY = -1;
		}

		void updateKernel(void)
		{
			m_kernel = getStructuringElement(m_kShape, cv::Size(m_kW, m_kH), cv::Point(m_aX, m_aY));
		}
	};

	class _Morphology : public _VisionBase
	{
	public:
		_Morphology();
		virtual ~_Morphology();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Morphology *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		Frame m_fIn;
		vector<IMG_MORPH> m_vFilter;
	};

}
#endif
