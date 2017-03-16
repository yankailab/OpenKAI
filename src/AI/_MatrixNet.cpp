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
	m_area.m_z = 1.0;
	m_area.m_w = 1.0;
	m_nObj = 0;
	m_ppObj = NULL;
}

_MatrixNet::~_MatrixNet()
{
}

bool _MatrixNet::init(void* pKiss)
{
	IF_F(!this->_AIbase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));

	F_INFO(pK->v("w", &m_w));
	F_INFO(pK->v("h", &m_h));
	F_INFO(pK->v("dW", &m_dW));
	F_INFO(pK->v("dH", &m_dH));
	F_INFO(pK->v("left", &m_area.m_x));
	F_INFO(pK->v("top", &m_area.m_y));
	F_INFO(pK->v("right", &m_area.m_z));
	F_INFO(pK->v("bottom", &m_area.m_w));

	return true;
}

bool _MatrixNet::link(void)
{
	IF_F(!this->_AIbase::link());
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

	int nObj = nW * nH;
	m_ppObj = new OBJECT*[nObj];

	OBJECT mO;
	int k = 0;
	double aW = m_area.m_z - m_area.m_x;
	double aH = m_area.m_w - m_area.m_y;

	for (int i = 0; i < nH; i++)
	{
		for (int j = 0; j < nW; j++)
		{
			mO.init();
			mO.m_fBBox.m_x = m_area.m_x + j * m_dW * aW;
			mO.m_fBBox.m_z = mO.m_fBBox.m_x + m_w * aW;
			mO.m_fBBox.m_y = m_area.m_y + i * m_dH * aH;
			mO.m_fBBox.m_w = mO.m_fBBox.m_y + m_h * aH;

			m_ppObj[k] = m_pIN->add(&mO);
			NULL_F(m_ppObj[k]);
			k++;
		}
	}

	m_nObj = nObj;

	return true;
}

void _MatrixNet::bSetActive(bool bActive)
{
	int i;
	for(i=0; i<m_nObj; i++)
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
					FONT_HERSHEY_SIMPLEX, m_sizeName, m_colName, 1);
		}

		Scalar colObs = m_colObs;
		rectangle(*pMat, r, colObs, 1);
	}

	return true;
}

}
