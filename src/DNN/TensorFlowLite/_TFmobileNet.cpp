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
		m_confidence = 0.25;
	}

	_TFmobileNet::~_TFmobileNet()
	{
	}

	int _TFmobileNet::init(void *pKiss)
	{
		CHECK_(this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nThreads", &m_nThreads);
		pK->v("confidence", &m_confidence);

		return OK_OK;
	}

	int _TFmobileNet::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _TFmobileNet::check(void)
	{
		NULL__(m_pU, OK_ERR_NULLPTR);
		NULL__(m_pV, OK_ERR_NULLPTR);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL__(pBGR, OK_ERR_NULLPTR);
		IF__(pBGR->bEmpty(), OK_ERR_NOT_READY);

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

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();
			m_pU->swap();

			ON_PAUSE;
		}
	}

	void _TFmobileNet::detect(void)
	{
		IF_(check() != OK_OK);

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
			o.setRect(x1, y1, x2 - x1, y2 - y1);
			// o.scale(kx, ky);

			m_pU->add(o);
			LOG_I("BBox: " << o.getText() << " Prob: " << f2str(o.getTopClassProb()));
		}
	}

}
