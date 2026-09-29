/*
 * RGBDframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__RGBDframe__H_
#define OpenKAI_src__DataStream__RGBDframe__H_

#include "DataObjBase.h"

namespace kai
{
	class RGBDframe : public DataObjBase
	{
	public:
		RGBDframe();
		virtual ~RGBDframe();
		void console(void *pConsole) override;

		void set(const Mat &rgb, const Mat &depth, uint64_t tStamp = 0);
		uint64_t get(Mat &rgb, Mat &depth);

	private:
		Mat m_mRGB;
		Mat m_mD;
		std::shared_mutex m_sMutex;
	};

}
#endif
