/*
 *  Created on: July 15, 2023
 *      Author: yankai
 */
#include "_TFmobileNet.h"

namespace kai
{

	_TFmobileNet::_TFmobileNet()
	{
		m_vSize.set(300, 300);
	}

	_TFmobileNet::~_TFmobileNet()
	{
	}

	bool _TFmobileNet::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		// pK->v("thr", &m_thr);

		return true;
	}

	bool _TFmobileNet::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _TFmobileNet::check(void)
	{
		NULL__(m_pU, -1);
		NULL__(m_pV, -1);
		Frame *pBGR = m_pV->BGR();
		NULL__(pBGR, -1);
		IF__(pBGR->bEmpty(), -1);

		return this->_DetectorBase::check();
	}

	bool _TFmobileNet::open(void)
	{
		// Load model
		m_model = tflite::FlatBufferModel::BuildFromFile(m_fModel.c_str());

		// Build the interpreter
		tflite::ops::builtin::BuiltinOpResolver resolver;
		tflite::InterpreterBuilder(*m_model.get(), resolver)(&m_interpreter);

		m_interpreter->AllocateTensors();

		return true;
	}

	void _TFmobileNet::update(void)
	{
		open();

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			detect();
			m_pU->swap();

			m_pT->autoFPSto();
		}
	}

	void _TFmobileNet::detect(void)
	{
		IF_(check() < 0);

		m_fBGR.copy(*m_pV->BGR());
		Mat image;
		cv::resize(*m_fBGR.m(), image, Size(m_vSize.x, m_vSize.y));
		memcpy(m_interpreter->typed_input_tensor<uchar>(0), image.data, image.total() * image.elemSize());

		m_interpreter->SetAllowFp16PrecisionForFp32(true);
		m_interpreter->SetNumThreads(4); // quad core
		m_interpreter->Invoke();		 // run your model

		const float *detection_locations = m_interpreter->tensor(m_interpreter->outputs()[0])->data.f;
		const float *detection_classes = m_interpreter->tensor(m_interpreter->outputs()[1])->data.f;
		const float *detection_scores = m_interpreter->tensor(m_interpreter->outputs()[2])->data.f;
		const int num_detections = *m_interpreter->tensor(m_interpreter->outputs()[3])->data.f;

		// there are ALWAYS 10 detections no matter how many objects are detectable
		//        cout << "number of detections: " << num_detections << "\n";

		const float confidence_threshold = 0.5;
		for (int i = 0; i < num_detections; i++)
		{
			IF_CONT(detection_scores[i] < confidence_threshold);
			int det_index = (int)detection_classes[i] + 1;
			float y1 = detection_locations[4 * i];
			float x1 = detection_locations[4 * i + 1];
			float y2 = detection_locations[4 * i + 2];
			float x2 = detection_locations[4 * i + 3];

			_Object o;
			o.init();
			o.setTstamp(m_pT->getTfrom());
			o.setTopClass(det_index, detection_scores[i]);
			string txt(getClassName(det_index)); // -1??
			o.setText(txt);
			o.setBB2D(x1, y1, x2 - x1, y2 - y1);
			// o.scale(kx, ky);

			m_pU->add(o);
			LOG_I("BBox: " << o.getText() << " Prob: " << f2str(o.getTopClassProb()));

			// Rect rec((int)x1, (int)y1, (int)(x2 - x1), (int)(y2 - y1));
			// rectangle(src, rec, Scalar(0, 0, 255), 1, 8, 0);
			// putText(src, format("%s", Labels[det_index].c_str()), Point(x1, y1 - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1, 8, 0);
		}
	}

}
