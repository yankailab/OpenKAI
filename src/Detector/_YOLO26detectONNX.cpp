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
		m_vModelInputSize = Vector2i(640, 640);
		m_vClass = vector<string>{"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};
	}

	_YOLO26detectONNX::~_YOLO26detectONNX()
	{
		DEL(m_pSession);
	}

	bool _YOLO26detectONNX::loadConfig(void)
	{
		IF_F(!this->_DetectorBase::loadConfig());
		const json &j = *m_pJ;

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

	bool _YOLO26detectONNX::saveConfig(bool bExport)
	{
		IF_F(!_DetectorBase::saveConfig(false));

		json &j = *m_pJ;
		j["confidence"] = m_confidence;
		j["score"] = m_score;
		j["nms"] = m_nms;
		j["bLetterBoxForSquare"] = m_bLetterBoxForSquare;
		j["vModelInputSize"] = {m_vModelInputSize.x(), m_vModelInputSize.y()};
		j["bSwapRB"] = m_bSwapRB;
		j["scale"] = m_scale;
		j["nThread"] = m_nThread;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
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

		return this->_DetectorBase::check();
	}

	void _YOLO26detectONNX::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
		}
	}

	void _YOLO26detectONNX::detect(void)
	{
		IF_(!check());

		Mat input;
		uint64_t tStamp = m_pRGBin->get(input);
		IF_(tStamp == m_tLastInput);
		m_tLastInput = tStamp;
		IF_(input.empty());
		m_pBBout->setContainerDim(Vector3f(input.cols, input.rows, 0));
		const Mat mIn = m_bLetterBoxForSquare && m_vModelInputSize.x() == m_vModelInputSize.y()
			? formatToSquare(input) : input;

		Mat mResized;
		cv::resize(mIn, mResized, cv::Size(m_vModelInputSize.x(), m_vModelInputSize.y()));

		vector<float> vTensor;
		matToTensor(mResized, &vTensor);

		vector<int64_t> vInputShape = {1, 3, m_vModelInputSize.y(), m_vModelInputSize.x()};
		Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
			m_memoryInfo,
			vTensor.data(),
			vTensor.size(),
			vInputShape.data(),
			vInputShape.size());

		const char *pInputName[] = {m_inputName.c_str()};
		const char *pOutputName[] = {m_outputName.c_str()};
		vector<BBOX_OBJ> vBB;

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

			if (!parseEnd2End(pData, vShape, mIn, vBB))
				parseOneToMany(pData, vShape, mIn, vBB);
		}
		catch (const Ort::Exception &e)
		{
			LOG_E("ONNX Runtime inference failed: " + string(e.what()));
			return;
		}

		// Letterboxing pads the bottom/right; publish only boxes within the source image.
		vBB.erase(std::remove_if(vBB.begin(), vBB.end(), [&input](BBOX_OBJ &bb)
		{
			float left = std::max(0.0f, bb.m_vPos.x());
			float top = std::max(0.0f, bb.m_vPos.y());
			float right = std::min(static_cast<float>(input.cols), bb.m_vPos.x() + bb.m_vDim.x());
			float bottom = std::min(static_cast<float>(input.rows), bb.m_vPos.y() + bb.m_vDim.y());
			if (right <= left || bottom <= top)
				return true;

			bb.setPos(Vector3f(left, top, 0));
			bb.setDim(Vector3f(right - left, bottom - top, 0));
			return false;
		}), vBB.end());

		m_pBBout->add(vBB, tStamp);
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

		pvTensor->resize(3 * m_vModelInputSize.x() * m_vModelInputSize.y());
		size_t nChannel = m_vModelInputSize.x() * m_vModelInputSize.y();
		for (int i = 0; i < 3; i++)
			memcpy(pvTensor->data() + i * nChannel, vCHW[i].data, nChannel * sizeof(float));
	}

	bool _YOLO26detectONNX::parseEnd2End(float *pData, const vector<int64_t> &vShape, const Mat &mIn, vector<BBOX_OBJ> &vBB)
	{
		NULL_F(pData);
		IF_F(vShape.size() != 3);
		IF_F(vShape[2] < 6);
		IF_F(vShape[1] > 1000);

		float kx = (float)mIn.cols / (float)m_vModelInputSize.x();
		float ky = (float)mIn.rows / (float)m_vModelInputSize.y();

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

			BBOX_OBJ bb;
			bb.setType(obj_bbox);
			bb.setPos(Vector3f(left, top, 0));
			bb.setDim(Vector3f(right - left, bottom - top, 0));
			bb.addClass(iClass, m_vClass[iClass], static_cast<int8_t>(std::max(0.0f, std::min(1.0f, confidence)) * 100.0f + 0.5f));

			vBB.push_back(bb);
			LOG_I("Class: " + i2str(bb.getTopClassID()));
		}

		return true;
	}

	bool _YOLO26detectONNX::parseOneToMany(float *pData, const vector<int64_t> &vShape, const Mat &mIn, vector<BBOX_OBJ> &vBB)
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

		float kx = (float)mIn.cols / (float)m_vModelInputSize.x();
		float ky = (float)mIn.rows / (float)m_vModelInputSize.y();

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

		for (unsigned long i = 0; i < nmsResult.size(); i++)
		{
			int idx = nmsResult[i];

			const cv::Rect &r = vBox[idx];
			BBOX_OBJ bb;
			bb.setType(obj_bbox);
			bb.setPos(Vector3f(r.x, r.y, 0));
			bb.setDim(Vector3f(r.width, r.height, 0));
			bb.addClass(vClassID[idx], m_vClass[vClassID[idx]], static_cast<int8_t>(std::max(0.0f, std::min(1.0f, vConfidence[idx])) * 100.0f + 0.5f));

			vBB.push_back(bb);
			LOG_I("Class: " + i2str(bb.getTopClassID()));
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
