/*
 * RGBframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__RGBframe__H_
#define OpenKAI_src__DataStream__RGBframe__H_

#include "DataObjBase.h"

namespace kai
{
	class RGBframe : public DataObjBase
	{
	public:
		RGBframe();
		virtual ~RGBframe();
		void console(void *pConsole) override;

		void set(const Mat &src, uint64_t tStamp = 0);
		uint64_t get(Mat &dest);

	private:
		Mat m_mRGB;
		std::shared_mutex m_sMutex;
	};

}
#endif
