/*
 * _ARarea.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARarea.h"

#ifdef USE_OPENCV

namespace kai
{

	_ARarea::_ARarea()
	{
		m_pV = NULL;
		m_pD = NULL;
		m_pN = NULL;

		m_vRange.init(0.0, 100.0);
		m_vDoriginP.init(0, 0, 0);
		m_vDptW = {0, 0, 0};
		m_vCoriginP.init(0);
		m_vAxisIdx.init(0, 1, 2);
	}

	_ARarea::~_ARarea()
	{
	}

	bool _ARarea::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vRange", &m_vRange);
		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vCoriginA", &m_vCoriginP);

		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + " not found");

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pD = (_DistSensorBase *)(pK->getInst(n));
		IF_Fl(!m_pD, n + " not found");

		n = "";
		pK->v("_NavBase", &n);
		m_pN = (_NavBase *)(pK->getInst(n));
		IF_Fl(!m_pN, n + " not found");

		return true;
	}

	bool _ARarea::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARarea::check(void)
	{
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);
		NULL__(m_pD, -1);
		NULL__(m_pN, -1);

		return 0;
	}

	void _ARarea::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateARarea();

			m_pT->autoFPSto();
		}
	}

	bool _ARarea::updateARarea(void)
	{
		IF_F(check() < 0);

		m_d = m_pD->d((int)0);
		Vector3f vDptA = m_vDoriginP.v3f();
		vDptA[0] = m_d;

		Matrix4f mTpose = m_pN->mT();
		Eigen::Affine3f aTpose;
		aTpose = mTpose;
		m_vDptW = aTpose * vDptA;

		Matrix3f mR = m_pN->mR();
		Matrix3f mRt = mR.transpose();
		Matrix4f mTwc = Matrix4f::Identity();
		mTwc.block(0, 0, 3, 3) = mRt;
		mTwc(0, 3) = -mTpose(0, 3) + m_vCoriginP.x;
		mTwc(1, 3) = -mTpose(1, 3) + m_vCoriginP.y;
		mTwc(2, 3) = -mTpose(2, 3) + m_vCoriginP.z;

		m_aW2C = mTwc;

		return false;
	}

	bool _ARarea::w2c(const Vector3f &vPw,
					  const Eigen::Affine3f &aA,
					  float w,
					  float h,
					  vFloat2 &vF,
					  vFloat2 &vC,
					  vInt2 *pvPc)
	{
		NULL_F(pvPc);

		Vector3f vPcW = aA * vPw;
		Vector3f vP = Vector3f(
			vPcW[m_vAxisIdx.x],
			vPcW[m_vAxisIdx.y],
			vPcW[m_vAxisIdx.z]);

		float ovZ = 1.0 / vP[2];
		float x = w * (vF.x * vP[0] + vC.x * vP[2]) * ovZ;
		float y = h * (vF.y * vP[1] + vC.y * vP[2]) * ovZ;

		pvPc->x = x;
		pvPc->y = y;
	}

	void _ARarea::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		Mat mV;
		m_pV->BGR()->m()->copyTo(mV);
		float w = mV.cols;
		float h = mV.rows;
		vFloat2 vF = m_pV->getFf();
		vFloat2 vC = m_pV->getCf();

		vInt2 vPc;
		IF_(!w2c(m_vDptW, m_aW2C, w, h, vF, vC, &vPc));

		Scalar col = Scalar(0, 255, 0);

		circle(mV, Point(vPc.x, vPc.y), 5, col, 1);

		//Todo: limit to screen inside

		Mat m;
		cv::resize(mV, m, Size(pM->cols, pM->rows));
		m.copyTo(*pM);
	}

}
#endif
