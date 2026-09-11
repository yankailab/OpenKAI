/*
 * _YOLO26detectONNX.h
 *
 *  Created on: Jun 3, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__YOLO26detectONNX_H_
#define OpenKAI_src_Detector__YOLO26detectONNX_H_

#include "_DetectorBase.h"
#include <onnxruntime_cxx_api.h>
#include <algorithm>
#include <cstring>

namespace kai
{

	class _YOLO26detectONNX : public _DetectorBase
	{
	public:
		_YOLO26detectONNX();
		~_YOLO26detectONNX();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual bool loadModel(void);

	private:
		Mat formatToSquare(const Mat &mSrc);
		void detect(void);
		void matToTensor(const Mat &mSrc, vector<float> *pvTensor);
		bool parseEnd2End(float *pData, const vector<int64_t> &vShape, const Mat &mIn);
		bool parseOneToMany(float *pData, const vector<int64_t> &vShape, const Mat &mIn);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_YOLO26detectONNX *)This)->update();
			return NULL;
		}

	protected:
		Ort::Env m_env;
		Ort::SessionOptions m_sessionOptions;
		Ort::MemoryInfo m_memoryInfo;
		Ort::Session *m_pSession = NULL;

		string m_inputName;
		string m_outputName;

		float m_confidence = 0.25;
		float m_score = 0.45;
		float m_nms = 0.5;
		bool m_bLetterBoxForSquare = true;
		vInt2 m_vModelInputSize;
		bool m_bSwapRB = true;
		float m_scale = 1.0 / 255.0;
		int m_nThread = 1;
	};

}
#endif
