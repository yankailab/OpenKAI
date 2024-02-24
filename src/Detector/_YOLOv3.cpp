/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_YOLOv3.h"

namespace kai
{

	_YOLOv3::_YOLOv3()
	{
		m_thr = 0.5;
		m_nms = 0.4;
		m_vBlobSize.set(416, 416);
		m_bSwapRB = true;
		m_vMean.clear();
		m_scale = 1.0 / 255.0;
		m_iClassDraw = -1;

		m_iBackend = cv::dnn::DNN_BACKEND_OPENCV;
		m_iTarget = cv::dnn::DNN_TARGET_CPU;
	}

	_YOLOv3::~_YOLOv3()
	{
	}

	bool _YOLOv3::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("thr", &m_thr);
		pK->v("nms", &m_nms);
		pK->v("vBlobSize", &m_vBlobSize);
		pK->v("iBackend", &m_iBackend);
		pK->v("iTarget", &m_iTarget);
		pK->v("bSwapRB", &m_bSwapRB);
		pK->v("scale", &m_scale);
		pK->v("iClassDraw", &m_iClassDraw);
		pK->v("vMean", &m_vMean);

		m_net = readNetFromDarknet(m_fModel, m_fWeight);

		IF_Fl(m_net.empty(), "read Net failed");

		m_net.setPreferableBackend(m_iBackend);
		m_net.setPreferableTarget(m_iTarget);

		vector<int> outLayers = m_net.getUnconnectedOutLayers();
		vector<String> layersNames = m_net.getLayerNames();
		m_vLayerName.resize(outLayers.size());

		for (size_t i = 0; i < outLayers.size(); i++)
			m_vLayerName[i] = layersNames[outLayers[i] - 1];

		return true;
	}

	bool _YOLOv3::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _YOLOv3::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			if (check() >= 0)
			{
				detectYolo();

				if (m_pT->bGoSleep())
					m_pU->clear();
			}

			m_pT->autoFPSto();
		}
	}

	int _YOLOv3::check(void)
	{
		NULL__(m_pU, -1);
		NULL__(m_pV, -1);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL__(pBGR, -1);
		IF__(pBGR->bEmpty(), -1);
		IF__(pBGR->tStamp() <= m_fRGB.tStamp(), -1);

		return this->_DetectorBase::check();
	}

	void _YOLOv3::detectYolo(void)
	{
		Frame *pBGR = m_pV->getFrameRGB();
		m_fRGB.copy(*pBGR);
		Mat mIn = *m_fRGB.m();

		m_blob = blobFromImage(mIn, m_scale, Size(m_vBlobSize.x, m_vBlobSize.y),
							   Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);
		m_net.setInput(m_blob);

		// Runs the forward pass to get output of the output layers
		vector<Mat> vO;
		m_net.forward(vO, m_vLayerName);

		// Remove the bounding boxes with low confidence
		vector<int> vClassID;
		vector<float> vConfidence;
		vector<Rect> vRect;

		for (size_t i = 0; i < vO.size(); i++)
		{
			// Scan through all the bounding boxes output from the network and keep only the
			// ones with high confidence scores. Assign the box's class label as the class
			// with the highest score for the box.
			float *pData = (float *)vO[i].data;
			for (int j = 0; j < vO[i].rows; ++j, pData += vO[i].cols)
			{
				Mat mScore = vO[i].row(j).colRange(5, vO[i].cols);
				Point pClassID;
				double conf;

				// Get the value and location of the maximum score
				cv::minMaxLoc(mScore, 0, &conf, 0, &pClassID);
				IF_CONT(conf < m_thr);

				int cX = (int)(pData[0] * mIn.cols);
				int cY = (int)(pData[1] * mIn.rows);
				int w = (int)(pData[2] * mIn.cols);
				int h = (int)(pData[3] * mIn.rows);
				int l = cX - w / 2;
				int t = cY - h / 2;

				vClassID.push_back(pClassID.x);
				vConfidence.push_back((float)conf);
				vRect.push_back(Rect(l, t, w, h));
			}
		}

		// Perform non maximum suppression to eliminate redundant overlapping boxes with lower confidences
		vector<int> vIndex;
		NMSBoxes(vRect, vConfidence, m_thr, m_nms, vIndex);
		float kx = 1.0 / mIn.cols;
		float ky = 1.0 / mIn.rows;

		for (size_t i = 0; i < vIndex.size(); i++)
		{
			int idx = vIndex[i];

			_Object o;
			o.clear();
			//		o.m_tStamp = m_pT->getTfrom();
			o.setTopClass(vClassID[idx], (float)vConfidence[idx]);
			o.setBB2D(rect2BB<vFloat4>(vRect[idx]));
			o.scale(kx, ky);

			m_pU->add(o);
			LOG_I("Class: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

	void _YOLOv3::detect(void)
	{
		Frame *pBGR = m_pV->getFrameRGB();
		m_fRGB.copy(*pBGR);
		Mat mIn = *m_fRGB.m();

		m_blob = blobFromImage(mIn, m_scale, Size(m_vBlobSize.x, m_vBlobSize.y),
							   Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);

		m_net.setInput(m_blob, "data");
		cv::Mat mDet = m_net.forward("detection_out");
		cv::Mat dMat(mDet.size[2], mDet.size[3], CV_32F, mDet.ptr<float>());

		for (int i = 0; i < dMat.rows; i++)
		{
			float conf = dMat.at<float>(i, 2);
			IF_CONT(conf < m_thr);

			vFloat4 bb;
			bb.x = dMat.at<float>(i, 3);
			bb.y = dMat.at<float>(i, 4);
			bb.z = dMat.at<float>(i, 5);
			bb.w = dMat.at<float>(i, 6);
			bb.constrain(0.0, 1.0);

			_Object o;
			o.clear();
			//		o.m_tStamp = m_pT->getTfrom();
			o.setTopClass(0.0, 1.0);
			o.setBB2D(bb);

			m_pU->add(o);
			LOG_I("Class: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}
}
