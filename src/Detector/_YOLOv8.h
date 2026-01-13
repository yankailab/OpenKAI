/*
 * _YOLOv8.h
 *
 *  Created on: Feb 9, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__YOLOv8_H_
#define OpenKAI_src_Detector__YOLOv8_H_

#include "_DetectorBase.h"

using namespace cv::dnn;

namespace kai
{

	class _YOLOv8 : public _DetectorBase
	{
	public:
		_YOLOv8();
		~_YOLOv8();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual bool loadModel(void);

	private:
		Mat formatToSquare(const Mat &mSrc);
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_YOLOv8 *)This)->update();
			return NULL;
		}

	protected:
		cv::dnn::Net m_net;
		float m_confidence;
		float m_score;
		float m_nms;
		bool m_bLetterBoxForSquare;
		vInt2 m_vModelInputSize;
		bool m_bSwapRB;
		float m_scale;

		int m_iBackend;
		int m_iTarget;
	};

}
#endif
