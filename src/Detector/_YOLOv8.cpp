/*
 * _YOLOv8.h
 *
 *  Created on: Feb 9, 2024
 *      Author: yankai
 */
#include "_YOLOv8.h"

namespace kai
{

	_YOLOv8::_YOLOv8()
	{
		m_confidence = 0.25;
		m_score = 0.45;
		m_nms = 0.5;
		m_bLetterBoxForSquare = true;
		m_vModelInputSize.set(640, 640);
		m_vClass = vector<string>{"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};
		m_bSwapRB = true;
		m_scale = 1.0 / 255.0;

		m_iBackend = cv::dnn::DNN_BACKEND_OPENCV;
		m_iTarget = cv::dnn::DNN_TARGET_CPU;
	}

	_YOLOv8::~_YOLOv8()
	{
	}

	int _YOLOv8::init(const json& j)
	{
		CHECK_(this->_DetectorBase::init(j));

		= j.value("confidence", &m_confidence);
		= j.value("score", &m_score);
		= j.value("nms", &m_nms);
		= j.value("bLetterBoxForSquare", &m_bLetterBoxForSquare);
		= j.value("vModelInputSize", &m_vModelInputSize);
		= j.value("bSwapRB", &m_bSwapRB);
		= j.value("scale", &m_scale);
		= j.value("iBackend", &m_iBackend);
		= j.value("iTarget", &m_iTarget);

		IF__(!loadModel(), OK_ERR_INVALID_VALUE);

		return true;
	}

	bool _YOLOv8::loadModel(void)
	{
		m_net = cv::dnn::readNetFromONNX(m_fModel);
		if (m_net.empty())
		{
			LOG_E("readNetFromONNX failed");
			return false;
		}

		m_net.setPreferableBackend(m_iBackend);
		m_net.setPreferableTarget(m_iTarget);

		return true;
	}

	int _YOLOv8::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _YOLOv8::check(void)
	{
		NULL__(m_pU);
		NULL__(m_pV);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL__(pBGR);
		IF__(pBGR->bEmpty());
		IF__(pBGR->tStamp() <= m_fRGB.tStamp());

		return this->_DetectorBase::check();
	}

	void _YOLOv8::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
		}
	}

	void _YOLOv8::detect(void)
	{
		IF_(!check());

		Frame *pBGR = m_pV->getFrameRGB();
		m_fRGB.copy(*pBGR);
		Mat mIn = *m_fRGB.m();

		if (m_bLetterBoxForSquare && m_vModelInputSize.x == m_vModelInputSize.y)
			mIn = formatToSquare(mIn);

		Mat mBlob;
		cv::dnn::blobFromImage(mIn,
							   mBlob,
							   m_scale,
							   cv::Size(m_vModelInputSize.x, m_vModelInputSize.y),
							   cv::Scalar(),
							   m_bSwapRB,
							   false);
		m_net.setInput(mBlob);

		vector<Mat> vMout;
		m_net.forward(vMout, m_net.getUnconnectedOutLayersNames());

		int rows = vMout[0].size[1];
		int dimensions = vMout[0].size[2];

		// yolov5 has an output of shape (batchSize, 25200, 85) (Num classes + box[x,y,w,h] + confidence[c])
		// yolov8 has an output of shape (batchSize, 84,  8400) (Num classes + box[x,y,w,h])
		IF_(dimensions <= rows); // Check if the shape[2] is more than shape[1] (yolov8)

		rows = vMout[0].size[2];
		dimensions = vMout[0].size[1];
		vMout[0] = vMout[0].reshape(1, dimensions);
		cv::transpose(vMout[0], vMout[0]);

		float *pData = (float *)vMout[0].data;
		float kx = (float)mIn.cols / (float)m_vModelInputSize.x;
		float ky = (float)mIn.rows / (float)m_vModelInputSize.y;

		vector<int> vClassID;
		vector<float> vConfidence;
		vector<cv::Rect> vBox;

		for (int i = 0; i < rows; i++)
		{
			float *pClassScores = pData + 4;

			cv::Mat scores(1, m_vClass.size(), CV_32FC1, pClassScores);
			cv::Point class_id;
			double maxClassScore;

			cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

			if (maxClassScore > m_score)
			{
				vConfidence.push_back(maxClassScore);
				vClassID.push_back(class_id.x);

				float x = pData[0];
				float y = pData[1];
				float w = pData[2];
				float h = pData[3];

				int left = int((x - 0.5 * w) * kx);
				int top = int((y - 0.5 * h) * ky);
				int width = int(w * kx);
				int height = int(h * ky);

				vBox.push_back(cv::Rect(left, top, width, height));
			}

			pData += dimensions;
		}

		vector<int> nms_result;
		cv::dnn::NMSBoxes(vBox, vConfidence, m_score, m_nms, nms_result);

		kx = 1.0 / (float)mIn.cols;
		ky = 1.0 / (float)mIn.rows;
		for (unsigned long i = 0; i < nms_result.size(); i++)
		{
			int idx = nms_result[i];

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

		m_pU->swap();
	}

	Mat _YOLOv8::formatToSquare(const Mat &mSrc)
	{
		int col = mSrc.cols;
		int row = mSrc.rows;
		int _max = MAX(col, row);
		cv::Mat mResult = Mat::zeros(_max, _max, CV_8UC3);
		mSrc.copyTo(mResult(Rect(0, 0, col, row)));
		return mResult;
	}

	void _YOLOv8::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_DetectorBase::console(pConsole);
		IF_(!check());
	}
}
