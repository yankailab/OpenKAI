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

    pthread_mutex_init(&m_wakeupMutex, NULL);
}

_PCbase::~_PCbase()
{
   	pthread_mutex_destroy(&m_wakeupMutex);
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

	string n;

	n = "";
	pK->v("_PCbase", &n );
	m_pPCB = (_PCbase*) (pK->getInst( n ));

	n = "";
	pK->v("_PCviewer", &n );
	m_pViewer = (_PCviewer*) (pK->getInst( n ));

	if(m_pViewer)
		m_iV = m_pViewer->addGeometry();

	return true;
}

int _PCbase::check(void)
{
	return 0;
}

void _PCbase::getPC(PointCloud* pPC)
{
    NULL_(pPC);
    
	pthread_mutex_lock(&m_mutexPC);
    *pPC = *m_sPC.prev();
	pthread_mutex_unlock(&m_mutexPC);
}

void _PCbase::updatePC(void)
{
	pthread_mutex_lock(&m_mutexPC);
	m_sPC.update();
    m_sPC.next()->points_.clear();
    m_sPC.next()->colors_.clear();
    m_sPC.next()->normals_.clear();
	pthread_mutex_unlock(&m_mutexPC);
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
