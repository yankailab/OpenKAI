/*
 * _SSD.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__SSD_H_
#define OpenKAI_src_DNN_Darknet__SSD_H_

#include "_DetectorBase.h"

using namespace cv::dnn;

namespace kai
{
	class _SSD : public _DetectorBase
	{
	public:
		_SSD();
		~_SSD();

		virtual int init(void *pKiss);
		virtual int start(void);
		virtual int check(void);

	private:
		void detect(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_SSD *)This)->update();
			return NULL;
		}

	protected:
		cv::dnn::Net m_net;
		double m_thr;
		double m_nms;
		vInt2 m_vBlobSize;
		bool m_bSwapRB;
		float m_scale;
		vInt3 m_vMean;
		Mat m_blob;
		int m_iClassDraw;

		int m_iBackend;
		int m_iTarget;
	};

}
#endif
