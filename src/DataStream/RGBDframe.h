/*
 * RGBDframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__RGBDframe__H_
#define OpenKAI_src__DataStream__RGBDframe__H_

#include "DataStreamBase.h"

namespace kai
{
	class RGBDframe : public DataStreamBase
	{
	public:
		RGBDframe();
		virtual ~RGBDframe();
		void console(void *pConsole) override;

		void copyFrom(const Mat& srcRGB, const Mat& srcD);
		void copyTo(Mat& destRGB, Mat& destD);

	protected:
		Mat m_mRGB;
		Mat m_mD;

		std::shared_mutex m_sMutex;
	};

}
#endif
