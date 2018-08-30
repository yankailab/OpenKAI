/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ClusterNet.h"

namespace kai
{

_ClusterNet::_ClusterNet()
{
	m_pDet = NULL;

	m_w = 0.2;
	m_h = 0.2;
	m_dW = 0.2;
	m_dH = 0.2;
	m_area.init();
	m_area.z = 1.0;
	m_area.w = 1.0;
	m_nObj = 0;
	m_ppObj = NULL;
	m_size.init();
	m_aW = 1.0;
	m_aH = 1.0;

	bSetActive(false);
}

_ClusterNet::~_ClusterNet()
{
}

bool _ClusterNet::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, w);
	KISSm(pK, h);
	KISSm(pK, dW);
	KISSm(pK, dH);
	F_INFO(pK->v("l", &m_area.x));
	F_INFO(pK->v("t", &m_area.y));
	F_INFO(pK->v("r", &m_area.z));
	F_INFO(pK->v("b", &m_area.w));

	//link
	string iName = "";

	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_ObjectBase*) (pK->root()->getChildInstByName(&iName));
	NULL_Fl(m_pDet, "_DetectorBase not found");

	//create detection area instances
	m_size.x = ((1.0 - m_w) / m_dW) + 1;
	m_size.y = ((1.0 - m_h) / m_dH) + 1;
	if (m_size.x <= 0 || m_size.y <= 0)
	{
		LOG_E("nW <=0 || nH <= 0");
		return false;
	}

	int i,j,k;

	m_nObj = m_size.x * m_size.y;
	m_ppObj = new OBJECT*[m_nObj];
	for(i=0; i<m_nObj; i++)
		m_ppObj[i]=NULL;

	OBJECT mO;
	k = 0;
	m_aW = m_area.z - m_area.x;
	m_aH = m_area.w - m_area.y;

	for (i = 0; i < m_size.y; i++)
	{
		for (j = 0; j < m_size.x; j++)
		{
			mO.init();
			mO.m_bb.x = m_area.x + j * m_dW * m_aW;
			mO.m_bb.z = mO.m_bb.x + m_w * m_aW;
			mO.m_bb.y = m_area.y + i * m_dH * m_aH;
			mO.m_bb.w = mO.m_bb.y + m_h * m_aH;

			m_ppObj[k] = m_pDet->add(&mO);
			NULL_F(m_ppObj[k]);
			k++;
		}
	}

	m_pDet->m_obj.update();
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
//		m_ppObj[x]->m_bCluster = false;
	}

	for (y = 0; y < m_size.y; y++)
	{
		for (x = 0; x < m_size.x; x++)
		{
			OBJECT* pO = m_ppObj[m_size.x * y + x];
			IF_CONT(!pO);
			IF_CONT(pO->m_topClass < 0);

			vInt4 b = explore(x, y, pO->m_topClass);
			IF_CONT(b.x < 0);

			OBJECT o = *pO;
			o.m_bb.x = m_area.x + b.x * m_dW * m_aW;
			o.m_bb.z = m_area.x + (b.z * m_dW + m_w) * m_aW * m_aW;
			o.m_bb.y = m_area.y + b.y * m_dH * m_aH;
			o.m_bb.w = m_area.y + (b.w * m_dH + m_h) * m_aH * m_aH;
			add(&o);

			int i, j;
			for (j = b.y; j < b.w; j++)
			{
				for (i = b.x; i < b.z; i++)
				{
//					m_ppObj[m_size.x * j + i]->m_bCluster = true;
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
//	if (pO->m_bCluster)
//		return vB;
	if (pO->m_topClass != iClass)
		return vB;

//	pO->m_bCluster = true;
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

OBJECT* _ClusterNet::get(int i)
{
	IF_N(i >= m_nObj);

	return m_ppObj[i];
}

void _ClusterNet::bSetActive(bool bActive)
{
	m_bActive = bActive;

	NULL_(m_pDet);
	m_pDet->bSetActive(m_bActive);
}

bool _ClusterNet::bFound(int iClass)
{
	int i;
	for (i = 0; i < m_nObj; i++)
	{
		OBJECT* pObj = m_ppObj[i];
		IF_CONT(pObj->m_topClass != iClass);

		return true;
	}

	return false;
}

bool _ClusterNet::draw(void)
{
	IF_F(!this->_ObjectBase::draw());

	return true;
}

}
