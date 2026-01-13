/*
 * _YOLOv3.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__YOLOv3_H_
#define OpenKAI_src_DNN_Darknet__YOLOv3_H_

#include "_DetectorBase.h"

using namespace cv::dnn;

namespace kai
{
	class _YOLOv3 : public _DetectorBase
	{
	public:
		_YOLOv3();
		~_YOLOv3();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual bool check(void);

	private:
		void detectYolo(void);
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_YOLOv3 *)This)->update();
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
		vector<string> m_vLayerName;
		int m_iClassDraw;

		int m_iBackend;
		int m_iTarget;
	};

}
#endif
