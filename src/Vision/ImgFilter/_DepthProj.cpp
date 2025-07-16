/*
 * _DepthProj.cpp
 *
 *  Created on: June 15, 2022
 *      Author: yankai
 */

#include "_DepthProj.h"

namespace kai
{

	_DepthProj::_DepthProj()
	{
		m_type = vision_depthProj;
		m_pV = nullptr;
		m_fFov = 70;
		m_vCenter.set(0.5, 0.5);
	}

	_DepthProj::~_DepthProj()
	{
	}

	int _DepthProj::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("fFov", &m_fFov);
		pK->v("vCenter", &m_vCenter);

		return OK_OK;
	}

	int _DepthProj::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _DepthProj::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _DepthProj::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _DepthProj::filter(void)
	{
		NULL_(m_pV);
		Frame* pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());

		Mat mD = *pF->m();

		if ((m_mF.cols != mD.cols) || (m_mF.rows != mD.rows))
		{
			m_mF = Mat(mD.rows, mD.cols, mD.type(), Scalar(0));
			createFilterMat();
		}

		mD = mD.mul(m_mF);
		m_fRGB.copy(mD);
	}

	void _DepthProj::createFilterMat(void)
	{
		float hFov = m_fFov * 0.5 * DEG_2_RAD;
		vFloat2 vC;
		vC.set(m_vCenter.x * m_mF.cols,
			   m_vCenter.y * m_mF.rows);

		for (int i = 0; i < m_mF.rows; i++)
		{
			for (int j = 0; j < m_mF.cols; j++)
			{
				float fX = cos(2.0 * abs(j - vC.x) / (float)m_mF.cols);
				float fY = cos(2.0 * abs(i - vC.y) / (float)m_mF.rows);

				m_mF.at<float>(i, j) = fX * fY;
			}
		}
	}
}
