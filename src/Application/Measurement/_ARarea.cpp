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
		m_pS = NULL;

        m_vRange.init(0.0, 1000.0);
		m_vDoriginA.init(0,0,0);
		m_vDptW = {0,0,0};

        m_vCoriginA.init(0);
        m_mCoriginA = Matrix4f::Identity();

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

        pK->v("vCoriginA", &m_vCoriginA);
        m_mCoriginA = Matrix4f::Identity();
        m_mCoriginA(0, 3) = m_vCoriginA.x;
        m_mCoriginA(1, 3) = m_vCoriginA.y;
        m_mCoriginA(2, 3) = m_vCoriginA.z;

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
		pK->v("_SlamBase", &n);
		m_pS = (_SlamBase *)(pK->getInst(n));
		IF_Fl(!m_pS, n + " not found");

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
		NULL__(m_pS, -1);

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
		Vector3f vDptA = m_vDoriginA.v3f();
		vDptA[0] = m_d;

		Matrix4f mAtti = m_pS->mT().cast<float>();
		Eigen::Affine3f aAtti;
		aAtti = mAtti;
		m_vDptW = aAtti * vDptA;

		m_aW2C = m_mCoriginA * mAtti;

		vInt2 vPtScr;
		IF_F(!world2Scr(m_vDptW, m_aW2C, &vPtScr));
		

		return false;
	}

    bool _ARarea::world2Scr(const Vector3f &vPw, const Eigen::Affine3f &aA, vInt2* vPscr)
    {
		IF_F(check() < 0);
		NULL_F(vPscr);

        Vector3f vPcamW = aA * vPw;
        Vector3f vPcam = Vector3f(
            vPcamW[m_vAxisIdx.x],
            vPcamW[m_vAxisIdx.y],
            vPcamW[m_vAxisIdx.z]);

        Mat *pM = m_pV->BGR()->m();
        float w = pM->cols;
        float h = pM->rows;
        vDouble2 vFrgb = m_pV->getF();
        vDouble2 vCrgb = m_pV->getC();

        float ovZ = 1.0 / vPcam[2];
        float x = w * (vFrgb.x * vPcam[0] + vCrgb.x * vPcam[2]) * ovZ;
        float y = h * (vFrgb.y * vPcam[1] + vCrgb.y * vPcam[2]) * ovZ;

		//Todo: limit to screen inside

        Vec3b vC = pM->at<Vec3b>((int)y, (int)x);
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

		Scalar col = Scalar(0, 255, 0);

//		rectangle(*pM, r, col, 3);
	}

}
#endif
