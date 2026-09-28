/*
 * RGBframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__RGBframe__H_
#define OpenKAI_src__DataStream__RGBframe__H_

#include "DataStreamBase.h"

namespace kai
{
	class RGBframe : public DataStreamBase
	{
	public:
		RGBframe();
		virtual ~RGBframe();
		void console(void *pConsole) override;

		void copyFrom(const Mat& src);
		void copyTo(Mat& dest);

	protected:
		Mat m_mRGB;
		std::shared_mutex m_sMutex;
	};

}
#endif
