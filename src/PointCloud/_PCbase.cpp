/*
 * _PCbase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCbase.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCbase::_PCbase()
{
	m_pPCB = NULL;
	m_pViewer = NULL;
	m_iV = -1;
}

_PCbase::~_PCbase()
{
}

bool _PCbase::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	int nPCreserve = 0;
	pK->v("nPCreserve", &nPCreserve);
	if(nPCreserve > 0)
	{
		m_sPC.prev()->points_.reserve(nPCreserve);
		m_sPC.prev()->colors_.reserve(nPCreserve);
		m_sPC.next()->points_.reserve(nPCreserve);
		m_sPC.next()->colors_.reserve(nPCreserve);
	}

	string iName;

	iName = "";
	pK->v("_PCbase", &iName);
	m_pPCB = (_PCbase*) (pK->getInst(iName));

	iName = "";
	pK->v("_PCviewer", &iName);
	m_pViewer = (_PCviewer*) (pK->getInst(iName));

	if(m_pViewer)
		m_iV = m_pViewer->addGeometry();

	return true;
}

int _PCbase::check(void)
{
	return 0;
}

PointCloud* _PCbase::getPC(void)
{
	return m_sPC.prev();
}

int _PCbase::size(void)
{
	return m_sPC.prev()->points_.size();
}

void _PCbase::draw(void)
{
	this->_ThreadBase::draw();

//	NULL_(m_pViewer);
//	m_pViewer->updateGeometry(m_iV, getPC());
}

}
#endif
