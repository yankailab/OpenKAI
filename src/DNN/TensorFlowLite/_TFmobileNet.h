/*
 * _TFmobileNet.h
 *
 *  Created on: July 15, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_TensorFlowLite__TFmobileNet_H_
#define OpenKAI_src_DNN_TensorFlowLite__TFmobileNet_H_

#include "../../Vision/_VisionBase.h"
#include "../../Detector/_DetectorBase.h"

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/model.h"

namespace kai
{

	class _TFmobileNet : public _DetectorBase
	{
	public:
		_TFmobileNet();
		~_TFmobileNet();

		int init(void *pKiss);
		int start(void);
		int check(void);

	private:
		bool open(void);
		void detect(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_TFmobileNet *)This)->update();
			return NULL;
		}

	public:
		vInt2 m_vSize;
		std::vector<std::string> m_Labels;
		std::unique_ptr<tflite::Interpreter> m_interpreter;
		std::unique_ptr<tflite::FlatBufferModel> m_model;

		int m_nThreads;
		float m_confidence;
	};

}
#endif
