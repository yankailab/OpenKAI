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
	int nW = ((m_area.m_z - m_area.m_x - m_w) / m_dW) + 1;
	int nH = ((m_area.m_w - m_area.m_y - m_h) / m_dH) + 1;
	if (nW <= 0 || nH <= 0)
	{
		LOG_E("nW <=0 || nH <= 0");
		return false;
	}

	m_nObj = nW * nH;
	m_ppObj = new OBJECT*[m_nObj];

	OBJECT mO;
	int k = 0;
	for (int i = 0; i < nH; i++)
	{
		for (int j = 0; j < nW; j++)
		{
			mO.init();
			mO.m_fBBox.m_x = m_area.m_x + j * m_dW;
			mO.m_fBBox.m_z = m_area.m_x + mO.m_fBBox.m_x + m_w;
			mO.m_fBBox.m_y = m_area.m_y + i * m_dH;
			mO.m_fBBox.m_w = m_area.m_y + mO.m_fBBox.m_y + m_h;

			m_ppObj[k] = m_pIN->add(&mO);
			NULL_F(m_ppObj[k]);
			k++;
		}
	}

	return true;
}

void _MatrixNet::active(bool bActive)
{
	int i;
	for(i=0; i<m_nObj; i++)
	{

	}

}

OBJECT* _MatrixNet::get(int i, int64_t minFrameID)
{
	IF_N(m_ppObj[i]->m_frameID < minFrameID);

	return m_ppObj[i];
}

bool _MatrixNet::bFound(int iClass, double minProb)
{
	int i;
	for (i = 0; i < m_nObj; i++)
	{
		OBJECT* pObj = m_ppObj[i];
		IF_CONT(pObj->m_iClass != iClass);
		IF_CONT(pObj->m_prob < minProb);

		return true;
	}

	return false;
}

bool _MatrixNet::draw(void)
{
	IF_F(!this->_AIbase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
