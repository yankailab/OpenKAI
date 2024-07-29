/*
 * _DepthSegment.h
 *
 *  Created on: May 29, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__DepthSegment_H_
#define OpenKAI_src_Detector__DepthSegment_H_

#include "_DetectorBase.h"

namespace kai
{

	class _DepthSegment : public _DetectorBase
	{
	public:
		_DepthSegment();
		virtual ~_DepthSegment();

		int init(void *pKiss);
		int start(void);
		int check(void);
		void draw(void *pFrame);

	private:
		void detect(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_DepthSegment *)This)->update();
			return NULL;
		}

	protected:
		Mat m_mR;
		float m_rL;
		float m_rH;
		float m_rD;
		float m_rArea;
	};

}
#endif
