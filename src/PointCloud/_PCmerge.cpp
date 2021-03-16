/*
 * _PCmerge.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCmerge.h"

#ifdef USE_OPEN3D

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
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

    pK->v("rVoxel", &m_rVoxel);
    
	string n;

	vector<string> vPCB;
	pK->a("vPCbase", &vPCB);
	IF_F(vPCB.empty());

	for(string p : vPCB)
	{
		_PCbase* pPCB = (_PCbase*) (pK->getInst(p));
		IF_CONT(!pPCB);

		m_vpPCB.push_back(pPCB);
	}
	IF_F(m_vpPCB.empty());

	return true;
}

bool _PCmerge::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCmerge::check(void)
{
	return this->_PCbase::check();
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
	for(_PCbase* pPCB : m_vpPCB)
	{
		m_ring.readSrc(pPCB->getRing());
	}

}

}
#endif
