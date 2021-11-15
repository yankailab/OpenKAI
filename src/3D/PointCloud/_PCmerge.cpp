/*
 * _PCmerge.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCmerge.h"

namespace kai
{

_PCmerge::_PCmerge()
{
    m_rVoxel = 0.0;
}

_PCmerge::~_PCmerge()
{
}

bool _PCmerge::init(void *pKiss)
{
	IF_F(!_GeometryBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

    pK->v("rVoxel", &m_rVoxel);
    
	string n;

	vector<string> vPCB;
	pK->a("vPCbase", &vPCB);
	IF_F(vPCB.empty());

	for(string p : vPCB)
	{
		_GeometryBase* pPCB = (_GeometryBase*) (pK->getInst(p));
		IF_CONT(!pPCB);

		m_vpGB.push_back(pPCB);
	}
	IF_F(m_vpGB.empty());

	return true;
}

bool _PCmerge::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCmerge::check(void)
{
	return this->_GeometryBase::check();
}

void _PCmerge::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateMerge();
        
		m_pT->autoFPSto();
	}
}

void _PCmerge::updateMerge(void)
{
	IF_(check() < 0);

	//read all inputs into one ring
	for(_GeometryBase* pPCB : m_vpGB)
	{
//		m_ring.readSrc(pPCB->getRing());
	}

}

}
