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
	m_vMean.init();
	m_scale = 1.0;

	m_iBackend = cv::dnn::DNN_BACKEND_OPENCV;
	m_iTarget = cv::dnn::DNN_TARGET_CPU;
}

_DNNclassifier::~_DNNclassifier()
{
}

bool _DNNclassifier::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nW",&m_nW);
	pK->v("nH",&m_nH);
	pK->v("bSwapRB",&m_bSwapRB);
	pK->v("scale",&m_scale);
	pK->v("iBackend",&m_iBackend);
	pK->v("iTarget",&m_iTarget);
	pK->v("meanB",&m_vMean.x);
	pK->v("meanG",&m_vMean.y);
	pK->v("meanR",&m_vMean.z);

	Kiss* pR = pK->o("ROI");
	Kiss** pItr;
	vFloat4 r;
	if(pR)
	{
		pItr = pR->getChildItr();
		int i = 0;
		while ((pR = pItr[i++]))
		{
			r.init();
			pR->v("x", &r.x);
			pR->v("y", &r.y);
			pR->v("z", &r.z);
			pR->v("w", &r.w);
			m_vROI.push_back(r);
		}
	}

	if(m_vROI.empty())
	{
		r.init();
		r.z = 1.0;
		r.w = 1.0;
		m_vROI.push_back(r);
	}

	m_net = readNet(m_trainedFile, m_modelFile);
	IF_Fl(m_net.empty(), "read Net failed");

	m_net.setPreferableBackend(m_iBackend);
	m_net.setPreferableTarget(m_iTarget);

	return true;
}

bool _DNNclassifier::start(void)
{
	IF_T(m_threadMode != T_THREAD);

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DNNclassifier::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		IF_CONT(!classify());

		updateObj();

		if (m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _DNNclassifier::check(void)
{
	NULL__(m_pV, -1);
	Frame* pBGR = m_pV->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_fBGR.tStamp(), -1);

	return 0;
}

bool _DNNclassifier::classify(void)
{
	IF_F(check() < 0);
	Frame* pBGR = m_pV->BGR();
	m_fBGR.copy(*pBGR);
	Mat mIn = *m_fBGR.m();
	vInt2 cs;
	cs.x = mIn.cols;
	cs.y = mIn.rows;

	for(int i=0; i<m_vROI.size(); i++)
	{
		vFloat4 fBB = m_vROI[i];
		Rect r = convertBB(convertBB(fBB, cs));

		m_blob = blobFromImage(mIn(r),
								m_scale,
								Size(m_nW, m_nH),
								Scalar(m_vMean.z, m_vMean.y, m_vMean.x),
								m_bSwapRB,
								false);
		m_net.setInput(m_blob);

		Mat mProb = m_net.forward();

	    Point pClassID;
	    double conf;
	    cv::minMaxLoc(mProb.reshape(1, 1), 0, &conf, 0, &pClassID);

		OBJECT o;
		o.init();
		o.m_tStamp = m_tStamp;
		o.setTopClass(pClassID.x, conf);
		o.m_bb = fBB;
		this->add(&o);

		LOG_I("Class: " + i2str(o.m_topClass));
	}

	return true;
}

bool _DNNclassifier::classify(Mat m, OBJECT* pO)
{
	IF_F(m.empty());
	NULL_F(pO);

	m_blob = blobFromImage(m,
							m_scale,
							Size(m_nW, m_nH),
							Scalar(m_vMean.z, m_vMean.y, m_vMean.x),
							m_bSwapRB,
							false);
	m_net.setInput(m_blob);

	Mat mProb = m_net.forward();

    Point pClassID;
    double conf;
    cv::minMaxLoc(mProb.reshape(1, 1), 0, &conf, 0, &pClassID);
    IF_F(conf < m_minConfidence);

	pO->setTopClass(pClassID.x, conf);

	return true;
}

void _DNNclassifier::draw(void)
{
	this->_DetectorBase::draw();

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	OBJECT* pO = at(0);
	NULL_(pO);

	int iClass = pO->m_topClass;
	IF_(iClass >= m_nClass);
	IF_(iClass < 0);

	string oName = m_vClass[iClass].m_name;
	if (oName.length()>0)
	{
		putText(*pMat, oName,
				Point(25, 100),
				FONT_HERSHEY_SIMPLEX, 2.0, Scalar(0,0,255), 5);
	}
}

}
