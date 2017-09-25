/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ClusterNet.h"

namespace kai
{

_ClusterNet::_ClusterNet()
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
	m_size.init();
	m_aW = 1.0;
	m_aH = 1.0;
	m_bActive = false;
}

_ClusterNet::~_ClusterNet()
{
}

bool _ClusterNet::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));

	KISSm(pK, w);
	KISSm(pK, h);
	KISSm(pK, dW);
	KISSm(pK, dH);
	F_INFO(pK->v("l", &m_area.x));
	F_INFO(pK->v("t", &m_area.y));
	F_INFO(pK->v("r", &m_area.z));
	F_INFO(pK->v("b", &m_area.w));

	return true;
}

bool _ClusterNet::link(void)
{
	IF_F(!this->_DetectorBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	//create marker detection area instances
	m_size.x = ((1.0 - m_w) / m_dW) + 1;
	m_size.y = ((1.0 - m_h) / m_dH) + 1;
	if (m_size.x <= 0 || m_size.y <= 0)
	{
		LOG_E("nW <=0 || nH <= 0");
		return false;
	}

	m_nObj = m_size.x * m_size.y;
	m_ppObj = new OBJECT*[m_nObj];

	OBJECT mO;
	int k = 0;
	m_aW = m_area.z - m_area.x;
	m_aH = m_area.w - m_area.y;

	for (int i = 0; i < m_size.y; i++)
	{
		for (int j = 0; j < m_size.x; j++)
		{
			mO.init();
			mO.m_fBBox.x = m_area.x + j * m_dW * m_aW;
			mO.m_fBBox.z = mO.m_fBBox.x + m_w * m_aW;
			mO.m_fBBox.y = m_area.y + i * m_dH * m_aH;
			mO.m_fBBox.w = mO.m_fBBox.y + m_h * m_aH;

			m_ppObj[k] = m_pIN->add(&mO);
			NULL_F(m_ppObj[k]);
			k++;
		}
	}

	m_pIN->m_obj.update();
	bSetActive(true);

	return true;
}

bool _ClusterNet::start(void)
{
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

void _ClusterNet::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		IF_CONT(!m_bActive);

		cluster();

		m_obj.update();

		this->autoFPSto();
	}
}

void _ClusterNet::cluster(void)
{
	int y, x;
	for (x = 0; x < m_nObj; x++)
	{
		m_ppObj[x]->m_bCluster = false;
	}

	for (y = 0; y < m_size.y; y++)
	{
		for (x = 0; x < m_size.x; x++)
		{
			OBJECT* pO = m_ppObj[m_size.x * y + x];
			IF_CONT(!pO);
			IF_CONT(pO->m_iClass < 0);

			vInt4 b = explore(x, y, pO->m_iClass);
			IF_CONT(b.x < 0);

			OBJECT o = *pO;
			o.m_fBBox.x = m_area.x + b.x * m_dW * m_aW;
			o.m_fBBox.z = m_area.x + (b.z * m_dW + m_w) * m_aW * m_aW;
			o.m_fBBox.y = m_area.y + b.y * m_dH * m_aH;
			o.m_fBBox.w = m_area.y + (b.w * m_dH + m_h) * m_aH * m_aH;
			o.f2iBBox();
			o.m_prob = 1.0;
			add(&o);

			int i, j;
			for (j = b.y; j < b.w; j++)
			{
				for (i = b.x; i < b.z; i++)
				{
					m_ppObj[m_size.x * j + i]->m_bCluster = true;
				}
			}
		}
	}
}

vInt4 _ClusterNet::explore(int x, int y, int iClass)
{
	vInt4 vB;
	vB.x = -1;

	if (x < 0)
		return vB;
	if (y < 0)
		return vB;
	if (x >= m_size.x)
		return vB;
	if (y >= m_size.y)
		return vB;

	OBJECT* pO = m_ppObj[m_size.x * y + x];

	if (!pO)
		return vB;
	if (pO->m_bCluster)
		return vB;
	if (pO->m_iClass != iClass)
		return vB;

	pO->m_bCluster = true;
	vB.x = x;
	vB.y = y;
	vB.z = x;
	vB.w = y;

	vInt4 b;

	b = explore(x - 1, y, iClass);
	if (b.x >= 0)
	{
		vB.x = small(vB.x, b.x);
		vB.y = small(vB.y, b.y);
		vB.z = big(vB.z, b.z);
		vB.w = big(vB.w, b.w);
	}

	b = explore(x + 1, y, iClass);
	if (b.x >= 0)
	{
		vB.x = small(vB.x, b.x);
		vB.y = small(vB.y, b.y);
		vB.z = big(vB.z, b.z);
		vB.w = big(vB.w, b.w);
	}

	b = explore(x - 1, y + 1, iClass);
	if (b.x >= 0)
	{
		vB.x = small(vB.x, b.x);
		vB.y = small(vB.y, b.y);
		vB.z = big(vB.z, b.z);
		vB.w = big(vB.w, b.w);
	}

	b = explore(x, y + 1, iClass);
	if (b.x >= 0)
	{
		vB.x = small(vB.x, b.x);
		vB.y = small(vB.y, b.y);
		vB.z = big(vB.z, b.z);
		vB.w = big(vB.w, b.w);
	}

	b = explore(x + 1, y + 1, iClass);
	if (b.x >= 0)
	{
		vB.x = small(vB.x, b.x);
		vB.y = small(vB.y, b.y);
		vB.z = big(vB.z, b.z);
		vB.w = big(vB.w, b.w);
	}

	return vB;
}

void _ClusterNet::bSetActive(bool bActive)
{
	for (int i = 0; i < m_nObj; i++)
	{
		m_ppObj[i]->m_bClassify = bActive;
	}

	m_bActive = bActive;
}

OBJECT* _ClusterNet::get(int i)
{
	IF_N(i >= m_nObj);

	return m_ppObj[i];
}

bool _ClusterNet::bFound(int iClass, double minProb)
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

bool _ClusterNet::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}
