/*
 * _YOLO26detectONNX.cpp
 *
 *  Created on: Jun 3, 2026
 *      Author: yankai
 */
#include "_YOLO26detectONNX.h"

namespace kai
{

	_YOLO26detectONNX::_YOLO26detectONNX() : m_env(ORT_LOGGING_LEVEL_WARNING, "OpenKAI_YOLO26detectONNX"),
						 m_memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
	{
		m_pSession = NULL;
		m_confidence = 0.25;
		m_score = 0.45;
		m_nms = 0.5;
		m_bLetterBoxForSquare = true;
		m_vModelInputSize.set(640, 640);
		m_vClass = vector<string>{"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};
		m_bSwapRB = true;
		m_scale = 1.0 / 255.0;
		m_nThread = 1;
	}

	_YOLO26detectONNX::~_YOLO26detectONNX()
	{
		DEL(m_pSession);
	}

	bool _YOLO26detectONNX::init(const json &j)
	{
		IF_F(!this->_DetectorBase::init(j));

		jKv(j, "confidence", m_confidence);
		jKv(j, "score", m_score);
		jKv(j, "nms", m_nms);
		jKv(j, "bLetterBoxForSquare", m_bLetterBoxForSquare);
		jKv<int>(j, "vModelInputSize", m_vModelInputSize);
		jKv(j, "bSwapRB", m_bSwapRB);
		jKv(j, "scale", m_scale);
		jKv(j, "nThread", m_nThread);

		IF_F(!loadModel());

		return true;
	}

	bool _YOLO26detectONNX::loadModel(void)
	{
		DEL(m_pSession);

		try
		{
			m_sessionOptions.SetIntraOpNumThreads(m_nThread);
			m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
			m_pSession = new Ort::Session(m_env, m_fModel.c_str(), m_sessionOptions);

			Ort::AllocatorWithDefaultOptions allocator;
			Ort::AllocatedStringPtr inputName = m_pSession->GetInputNameAllocated(0, allocator);
			Ort::AllocatedStringPtr outputName = m_pSession->GetOutputNameAllocated(0, allocator);
			m_inputName = inputName.get();
			m_outputName = outputName.get();
		}
		catch (const Ort::Exception &e)
		{
			LOG_E("ONNX Runtime load failed: " + string(e.what()));
			return false;
		}

		return true;
	}

	bool _YOLO26detectONNX::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _YOLO26detectONNX::check(void)
	{
		NULL_F(m_pSession);
		NULL_F(m_pU);
		NULL_F(m_pV);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL_F(pBGR);
		IF_F(pBGR->bEmpty());
		IF_F(pBGR->tStamp() <= m_fRGB.tStamp());

		return this->_DetectorBase::check();
	}

	void _YOLO26detectONNX::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
		}
	}

	void _YOLO26detectONNX::detect(void)
	{
		IF_(!check());

		Frame *pBGR = m_pV->getFrameRGB();
		m_fRGB.copy(*pBGR);
		Mat mIn = *m_fRGB.m();

		if (m_bLetterBoxForSquare && m_vModelInputSize.x == m_vModelInputSize.y)
			mIn = formatToSquare(mIn);

		Mat mResized;
		cv::resize(mIn, mResized, cv::Size(m_vModelInputSize.x, m_vModelInputSize.y));

		vector<float> vTensor;
		matToTensor(mResized, &vTensor);

		vector<int64_t> vInputShape = {1, 3, m_vModelInputSize.y, m_vModelInputSize.x};
		Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
			m_memoryInfo,
			vTensor.data(),
			vTensor.size(),
			vInputShape.data(),
			vInputShape.size());

		const char *pInputName[] = {m_inputName.c_str()};
		const char *pOutputName[] = {m_outputName.c_str()};

		try
		{
			vector<Ort::Value> vOutput = m_pSession->Run(Ort::RunOptions{NULL},
														 pInputName,
														 &inputTensor,
														 1,
														 pOutputName,
														 1);

			float *pData = vOutput[0].GetTensorMutableData<float>();
			vector<int64_t> vShape = vOutput[0].GetTensorTypeAndShapeInfo().GetShape();

			if (!parseEnd2End(pData, vShape, mIn))
				parseOneToMany(pData, vShape, mIn);
		}
		catch (const Ort::Exception &e)
		{
			LOG_E("ONNX Runtime inference failed: " + string(e.what()));
			return;
		}

		m_pU->swap();
	}

	void _YOLO26detectONNX::matToTensor(const Mat &mSrc, vector<float> *pvTensor)
	{
		NULL_(pvTensor);

		Mat mFloat;
		mSrc.convertTo(mFloat, CV_32FC3, m_scale);

		if (m_bSwapRB)
			cv::cvtColor(mFloat, mFloat, cv::COLOR_BGR2RGB);

		vector<Mat> vCHW;
		cv::split(mFloat, vCHW);

		pvTensor->resize(3 * m_vModelInputSize.x * m_vModelInputSize.y);
		size_t nChannel = m_vModelInputSize.x * m_vModelInputSize.y;
		for (int i = 0; i < 3; i++)
			memcpy(pvTensor->data() + i * nChannel, vCHW[i].data, nChannel * sizeof(float));
	}

	bool _YOLO26detectONNX::parseEnd2End(float *pData, const vector<int64_t> &vShape, const Mat &mIn)
	{
		NULL_F(pData);
		IF_F(vShape.size() != 3);
		IF_F(vShape[2] < 6);
		IF_F(vShape[1] > 1000);

		float kx = (float)mIn.cols / (float)m_vModelInputSize.x;
		float ky = (float)mIn.rows / (float)m_vModelInputSize.y;
		float kBBx = 1.0 / (float)mIn.cols;
		float kBBy = 1.0 / (float)mIn.rows;

		for (int i = 0; i < vShape[1]; i++)
		{
			float *pD = pData + i * vShape[2];
			float confidence = pD[4];
			IF_CONT(confidence < m_confidence);

			int iClass = (int)(pD[5] + 0.5);
			IF_CONT(iClass < 0 || iClass >= (int)m_vClass.size());

			int left = int(pD[0] * kx);
			int top = int(pD[1] * ky);
			int right = int(pD[2] * kx);
			int bottom = int(pD[3] * ky);

			left = std::max(0, std::min(left, mIn.cols - 1));
			top = std::max(0, std::min(top, mIn.rows - 1));
			right = std::max(0, std::min(right, mIn.cols - 1));
			bottom = std::max(0, std::min(bottom, mIn.rows - 1));
			IF_CONT(right <= left || bottom <= top);

			_Object o;
			o.clear();
			o.setTstamp(m_pT->getTfrom());
			o.setType(obj_bbox);
			o.setTopClass(iClass, confidence);
			o.setBB2D(vFloat4(left, top, right - left, bottom - top), kBBx, kBBy);
			o.setText(m_vClass[iClass]);

			m_pU->add(o);
			LOG_I("Class: " + i2str(o.getTopClass()));
		}

		return true;
	}

	bool _YOLO26detectONNX::parseOneToMany(float *pData, const vector<int64_t> &vShape, const Mat &mIn)
	{
		NULL_F(pData);
		IF_F(vShape.size() != 3);

		int nPrediction;
		int nDimension;
		bool bChannelFirst = vShape[1] < vShape[2];

		if (bChannelFirst)
		{
			nDimension = vShape[1];
			nPrediction = vShape[2];
		}
		else
		{
			nPrediction = vShape[1];
			nDimension = vShape[2];
		}

		IF_F(nDimension < 5);
		int nClass = nDimension - 4;
		IF_F(nClass <= 0);

		float kx = (float)mIn.cols / (float)m_vModelInputSize.x;
		float ky = (float)mIn.rows / (float)m_vModelInputSize.y;

		vector<int> vClassID;
		vector<float> vConfidence;
		vector<cv::Rect> vBox;

		for (int i = 0; i < nPrediction; i++)
		{
			float x = bChannelFirst ? pData[i] : pData[i * nDimension];
			float y = bChannelFirst ? pData[nPrediction + i] : pData[i * nDimension + 1];
			float w = bChannelFirst ? pData[2 * nPrediction + i] : pData[i * nDimension + 2];
			float h = bChannelFirst ? pData[3 * nPrediction + i] : pData[i * nDimension + 3];

			int iClass = -1;
			float maxClassScore = 0.0;
			for (int j = 0; j < nClass; j++)
			{
				float score = bChannelFirst ? pData[(j + 4) * nPrediction + i] : pData[i * nDimension + 4 + j];
				if (score > maxClassScore)
				{
					maxClassScore = score;
					iClass = j;
				}
			}

			if (maxClassScore > m_score && iClass >= 0 && iClass < (int)m_vClass.size())
			{
				int left = int((x - 0.5 * w) * kx);
				int top = int((y - 0.5 * h) * ky);
				int width = int(w * kx);
				int height = int(h * ky);

				vConfidence.push_back(maxClassScore);
				vClassID.push_back(iClass);
				vBox.push_back(cv::Rect(left, top, width, height));
			}
		}

		vector<int> nmsResult;
		cv::dnn::NMSBoxes(vBox, vConfidence, m_score, m_nms, nmsResult);

		kx = 1.0 / (float)mIn.cols;
		ky = 1.0 / (float)mIn.rows;
		for (unsigned long i = 0; i < nmsResult.size(); i++)
		{
			int idx = nmsResult[i];

			_Object o;
			o.clear();
			o.setTstamp(m_pT->getTfrom());
			o.setType(obj_bbox);
			o.setTopClass(vClassID[idx], vConfidence[idx]);
			o.setBB2D(rect2BB<vFloat4>(vBox[idx]), kx, ky);
			o.setText(m_vClass[vClassID[idx]]);

			m_pU->add(o);
			LOG_I("Class: " + i2str(o.getTopClass()));
		}

		return true;
	}

	Mat _YOLO26detectONNX::formatToSquare(const Mat &mSrc)
	{
		int col = mSrc.cols;
		int row = mSrc.rows;
		int _max = MAX(col, row);
		cv::Mat mResult = Mat::zeros(_max, _max, CV_8UC3);
		mSrc.copyTo(mResult(Rect(0, 0, col, row)));
		return mResult;
	}

	void _YOLO26detectONNX::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_DetectorBase::console(pConsole);
		IF_(!check());
	}
}
