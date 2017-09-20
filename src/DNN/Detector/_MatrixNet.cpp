/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_MatrixNet.h"

namespace kai
{

_MatrixNet::_MatrixNet()
{
	m_pIN = NULL;
	m_w = 0.2;
	m_h = 0.2;
	m_dW = 0.5;
	m_dH = 0.5;
	m_area.init();
	m_area.z = 1.0;
	m_area.w = 1.0;
	m_nObj = 0;
	m_ppObj = NULL;
}

_MatrixNet::~_MatrixNet()
{
}

bool _MatrixNet::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));

	KISSm(pK,w);
	KISSm(pK,h);
	KISSm(pK,dW);
	KISSm(pK,dH);
	F_INFO(pK->v("l", &m_area.x));
	F_INFO(pK->v("t", &m_area.y));
	F_INFO(pK->v("r", &m_area.z));
	F_INFO(pK->v("b", &m_area.w));

	return true;
}

bool _MatrixNet::link(void)
{
	IF_F(!this->_DetectorBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	//create marker detection area instances
	int nW = ((1.0 - m_w) / m_dW) + 1;
	int nH = ((1.0 - m_h) / m_dH) + 1;
	if (nW <= 0 || nH <= 0)
	{
		LOG_E("nW <=0 || nH <= 0");
		return false;
	}

	m_nObj = nW * nH;
	m_ppObj = new OBJECT*[m_nObj];

	OBJECT mO;
	int k = 0;
	double aW = m_area.z - m_area.x;
	double aH = m_area.w - m_area.y;

	for (int i = 0; i < nH; i++)
	{
		for (int j = 0; j < nW; j++)
		{
			mO.init();
			mO.m_fBBox.x = m_area.x + j * m_dW * aW;
			mO.m_fBBox.z = mO.m_fBBox.x + m_w * aW;
			mO.m_fBBox.y = m_area.y + i * m_dH * aH;
			mO.m_fBBox.w = mO.m_fBBox.y + m_h * aH;

			m_ppObj[k] = m_pIN->add(&mO);
			NULL_F(m_ppObj[k]);
			k++;
		}
	}

	return true;
}

void _MatrixNet::bSetActive(bool bActive)
{
	for(int i=0; i<m_nObj; i++)
	{
		m_ppObj[i]->m_bClassify = bActive;
	}
}

OBJECT* _MatrixNet::get(int i, int64_t minFrameID)
{
	IF_N(m_ppObj[i]->m_frameID < minFrameID);

	return m_ppObj[i];
}

bool _MatrixNet::bFound(int iClass, double minProb, int64_t minFrameID)
{
	int i;
	for (i = 0; i < m_nObj; i++)
	{
		OBJECT* pObj = m_ppObj[i];
		IF_CONT(pObj->m_iClass != iClass);
		IF_CONT(pObj->m_prob < minProb);
		IF_CONT(pObj->m_frameID < minFrameID);

		return true;
	}

	return false;
}

bool _MatrixNet::draw(void)
{
	IF_F(!this->_ThreadBase::draw());

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();
	IF_F(pMat->empty());

	for (int i = 0; i < m_nObj; i++)
	{
		OBJECT* pObj = get(i, 0);
		IF_CONT(!pObj);
		IF_CONT(pObj->m_frameID<=0)

		Rect r;
		vInt42rect(&pObj->m_bbox, &r);

		if (pObj->m_iClass>=0)
		{
			putText(*pMat, pObj->m_name,
					Point(r.x + r.width / 2, r.y + r.height / 2),
					FONT_HERSHEY_SIMPLEX, m_sizeName, m_col, 1);
		}

		Scalar colObs = m_col;
		rectangle(*pMat, r, colObs, 1);
	}

	return true;
}

}
