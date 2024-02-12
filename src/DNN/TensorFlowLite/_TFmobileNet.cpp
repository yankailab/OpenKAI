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
		m_nThreads = 4;
	}

	_TFmobileNet::~_TFmobileNet()
	{
	}

	bool _TFmobileNet::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nThreads", &m_nThreads);

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
		Frame *pBGR = m_pV->getFrameRGB();
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

		m_fRGB.copy(*m_pV->getFrameRGB());
		Mat m;
		cv::resize(*m_fRGB.m(), m, Size(m_vSize.x, m_vSize.y));
		if (m.channels() < 3)
			cv::cvtColor(m, m, COLOR_GRAY2RGB);
		memcpy(m_interpreter->typed_input_tensor<uchar>(0), m.data, m.total() * m.elemSize());

		m_interpreter->SetAllowFp16PrecisionForFp32(true);
		m_interpreter->SetNumThreads(m_nThreads);
		m_interpreter->Invoke();

		const float *detection_locations = m_interpreter->tensor(m_interpreter->outputs()[0])->data.f;
		const float *detection_classes = m_interpreter->tensor(m_interpreter->outputs()[1])->data.f;
		const float *detection_scores = m_interpreter->tensor(m_interpreter->outputs()[2])->data.f;
		const int num_detections = *m_interpreter->tensor(m_interpreter->outputs()[3])->data.f;

		// there are ALWAYS 10 detections no matter how many objects are detectable
		//        cout << "number of detections: " << num_detections << "\n";

		for (int i = 0; i < num_detections; i++)
		{
			IF_CONT(detection_scores[i] < m_confidence);
			int det_index = (int)detection_classes[i] + 1;
			float y1 = detection_locations[4 * i];
			float x1 = detection_locations[4 * i + 1];
			float y2 = detection_locations[4 * i + 2];
			float x2 = detection_locations[4 * i + 3];

			_Object o;
			o.clear();
			o.setTstamp(m_pT->getTfrom());
			o.setTopClass(det_index, detection_scores[i]);
			string txt(getClassName(det_index)); // -1??
			o.setText(txt);
			o.setBB2D(x1, y1, x2 - x1, y2 - y1);
			// o.scale(kx, ky);

			m_pU->add(o);
			LOG_I("BBox: " << o.getText() << " Prob: " << f2str(o.getTopClassProb()));
		}
	}

}
