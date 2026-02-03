/*
 *  Created on: Jan 13, 2019
 *      Author: yankai
 */
#include "_DNNclassifier.h"

namespace kai
{

	_DNNclassifier::_DNNclassifier()
	{
		m_nW = 224;
		m_nH = 224;
		m_bSwapRB = true;
		m_vMean.clear();
		m_scale = 1.0;

		m_iBackend = cv::dnn::DNN_BACKEND_OPENCV;
		m_iTarget = cv::dnn::DNN_TARGET_CPU;
	}

	_DNNclassifier::~_DNNclassifier()
	{
	}

	bool _DNNclassifier::init(const json &j)
	{
		IF_F(!this->_DetectorBase::init(j));

		jKv(j, "nW", m_nW);
		jKv(j, "nH", m_nH);
		jKv(j, "bSwapRB", m_bSwapRB);
		jKv(j, "scale", m_scale);
		jKv(j, "iBackend", m_iBackend);
		jKv(j, "iTarget", m_iTarget);
		jKv<int>(j, "vMeanBGR", m_vMean);

		const json &jF = jK(j, "vFilter");
		IF_F(!jF.is_object());

		vFloat4 r;
		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			r.clear();
			jKv<float>(Ji, "vROI", r);
			m_vROI.push_back(r);
		}

		if (m_vROI.empty())
		{
			r.clear();
			r.z = 1.0;
			r.w = 1.0;
			m_vROI.push_back(r);
		}

		m_net = readNet(m_fWeight, m_fModel);
		IF_F(m_net.empty());

		m_net.setPreferableBackend(m_iBackend);
		m_net.setPreferableTarget(m_iTarget);

		return true;
	}

	bool _DNNclassifier::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _DNNclassifier::check(void)
	{
		NULL_F(m_pU);
		NULL_F(m_pV);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL_F(pBGR);
		IF_F(pBGR->bEmpty());
		IF_F(pBGR->tStamp() <= m_fRGB.tStamp());

		return this->_DetectorBase::check();
	}

	void _DNNclassifier::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			classify();

			ON_PAUSE;
		}
	}

	void _DNNclassifier::classify(void)
	{
		IF_(!check());

		Frame *pBGR = m_pV->getFrameRGB();
		m_fRGB.copy(*pBGR);
		Mat m = *m_fRGB.m();

		for (int i = 0; i < m_vROI.size(); i++)
		{
			vFloat4 nBB = m_vROI[i];
			vFloat4 fBB = nBB;
			fBB.x *= m.cols;
			fBB.y *= m.rows;
			fBB.z *= m.cols;
			fBB.w *= m.rows;
			Rect r = bb2Rect(fBB);

			m_blob = blobFromImage(m(r), m_scale, Size(m_nW, m_nH),
								   Scalar(m_vMean.z, m_vMean.y, m_vMean.x), m_bSwapRB, false);
			m_net.setInput(m_blob);

			Mat mProb = m_net.forward();

			Point pClassID;
			double conf;
			cv::minMaxLoc(mProb.reshape(1, 1), 0, &conf, 0, &pClassID);

			_Object o;
			o.clear();
			//		o.m_tStamp = m_pT->getTfrom();
			o.setTopClass(pClassID.x, conf);
			o.setBB2D(nBB);
			m_pU->add(o);

			LOG_I("Class: " + i2str(o.getTopClass()));
		}

		m_pU->swap();
	}

	bool _DNNclassifier::classify(Mat m, _Object *pO, float minConfidence)
	{
		IF_F(m.empty());
		NULL_F(pO);

		m_blob = blobFromImage(m, m_scale, Size(m_nW, m_nH),
							   Scalar(m_vMean.z, m_vMean.y, m_vMean.x), m_bSwapRB, false);
		m_net.setInput(m_blob);

		Mat mProb = m_net.forward();

		Point pClassID;
		double conf;
		cv::minMaxLoc(mProb.reshape(1, 1), 0, &conf, 0, &pClassID);
		IF_F(conf < minConfidence);

		pO->setTopClass(pClassID.x, conf);
		return true;
	}

	void _DNNclassifier::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(!check());

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		_Object *pO = m_pU->get(0);
		NULL_(pO);

		int iClass = pO->getTopClass();
		IF_(iClass >= m_vClass.size());
		IF_(iClass < 0);

		string oName = m_vClass[iClass];
		if (oName.length() > 0)
		{
			putText(*pM, oName, Point(25, 100), FONT_HERSHEY_SIMPLEX, 2.0,
					Scalar(0, 0, 255), 5);
		}
	}

}
