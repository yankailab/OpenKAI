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
	m_pViewer = NULL;

}

_PCmerge::~_PCmerge()
{
}

bool _PCmerge::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	string n;

	vector<string> vPCB;
	pK->a("vPointCloudBase", &vPCB);
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
	IF_F(!this->_ModuleBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _PCmerge::check(void)
{
	return 0;
}

void _PCmerge::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateMerge();
		m_sPC.update();

		m_pT->autoFPSto();
	}
}

void _PCmerge::updateMerge(void)
{
	IF_(check() < 0);

	PointCloud* pOut = m_sPC.next();
	pOut->points_.clear();
	pOut->colors_.clear();
	pOut->normals_.clear();

	for(_PCbase* pPCB : m_vpPCB)
	{
        PointCloud pc;
        pPCB->getPC(&pc);
		IF_CONT(pc.points_.empty());

		vector<Eigen::Vector3d>* pV;
		pV = &pc.points_;
		pOut->points_.insert(pOut->points_.end(), pV->begin(), pV->end());

		pV = &pc.colors_;
		pOut->colors_.insert(pOut->colors_.end(), pV->begin(), pV->end());
	}

//	m_pViewer->render(pOut);
}

}
#endif
