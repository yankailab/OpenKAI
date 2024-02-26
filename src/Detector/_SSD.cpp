/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_SSD.h"

namespace kai
{

	_SSD::_SSD()
	{
		m_thr = 0.5;
		m_nms = 0.4;
		m_vBlobSize.set(300, 300);
		m_bSwapRB = true;
		m_vMean.set(127.5, 127.5, 127.5);
		m_scale = 1.0 / 255.0;
		m_iClassDraw = -1;

		m_iBackend = cv::dnn::DNN_BACKEND_OPENCV;
		m_iTarget = cv::dnn::DNN_TARGET_CPU;
	}

	_SSD::~_SSD()
	{
	}

	bool _SSD::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		pK->m_pInst = this;

		pK->v("thr", &m_thr);
		pK->v("nms", &m_nms);
		pK->v("vBlobSize", &m_vBlobSize);
		pK->v("iBackend", &m_iBackend);
		pK->v("iTarget", &m_iTarget);
		pK->v("bSwapRB", &m_bSwapRB);
		pK->v("scale", &m_scale);
		pK->v("iClassDraw", &m_iClassDraw);
		pK->v("vMean", &m_vMean);

		m_net = readNet(m_fWeight, m_fModel, "TensorFlow");
		IF_Fl(m_net.empty(), "read Net failed");

		m_net.setPreferableBackend(m_iBackend);
		m_net.setPreferableTarget(m_iTarget);

		return true;
	}

	bool _SSD::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _SSD::check(void)
	{
		NULL__(m_pU, -1);
		NULL__(m_pV, -1);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL__(pBGR, -1);
		IF__(pBGR->bEmpty(), -1);
		IF__(pBGR->tStamp() <= m_fRGB.tStamp(), -1);

		return this->_DetectorBase::check();
	}

	void _SSD::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			detect();

			ON_SLEEP;
			m_pT->autoFPSto();
		}
	}

	void _SSD::detect(void)
	{
		IF_(check() < 0);

		Frame *pBGR = m_pV->getFrameRGB();
		m_fRGB.copy(*pBGR);
		Mat mIn = *m_fRGB.m();

		m_blob = blobFromImage(mIn, m_scale, Size(m_vBlobSize.x, m_vBlobSize.y),
							   Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);

		m_net.setInput(m_blob);
		cv::Mat mDet = m_net.forward();
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
			//	o.m_tStamp = m_pT->getTfrom();
			o.setTopClass(dMat.at<float>(i, 1), 1.0);
			o.setBB2D(bb);

			m_pU->add(o);
			LOG_I("Class: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

}
