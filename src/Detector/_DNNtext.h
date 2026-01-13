/*
 * _DNNtext.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__DNNtext_H_
#define OpenKAI_src_DNN_Darknet__DNNtext_H_

#include "_YOLOv3.h"
#ifdef USE_OCR
#include "OCR.h"
#endif

namespace kai
{

	class _DNNtext : public _YOLOv3
	{
	public:
		_DNNtext();
		~_DNNtext();

		virtual bool init(const json& j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual void draw(void *pFrame);
		virtual bool check(void);
		void ocr(void);

	protected:
		Mat getTransform(Mat mImg, vFloat2 *pBox);
		void decode(const Mat &mScores, const Mat &mGeometry, float scoreThresh,
					std::vector<RotatedRect> &vDetections, std::vector<float> &vConfidences);
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_DNNtext *)This)->update();
			return NULL;
		}

	protected:
		vDouble3 m_vMean;
		bool m_bDetect;
		bool m_bOCR;
		bool m_bWarp;

#ifdef USE_OCR
		OCR *m_pOCR;
#endif
	};

}
#endif
