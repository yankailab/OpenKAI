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
    m_vColOvrr.x = -1.0;
    
    m_bTransform = false;
    m_vT.init(0);
	m_vR.init(0);
	m_A = Eigen::Matrix4d::Identity();

    pthread_mutex_init ( &m_mutexPC, NULL );
    
    m_pP = NULL;
    m_nP = 256;
    m_iP = 0;
    m_tP = 0;
}

_PCbase::~_PCbase()
{
    pthread_mutex_destroy ( &m_mutexPC );
}

bool _PCbase::init ( void *pKiss )
{
    IF_F ( !this->_ModuleBase::init ( pKiss ) );
    Kiss *pK = ( Kiss* ) pKiss;

    pK->v ( "vColOvrr", &m_vColOvrr );

    //frame
    int nPCreserve = 0;
    pK->v ( "nPCreserve", &nPCreserve );
    if ( nPCreserve > 0 )
    {
        m_sPC.prev()->points_.reserve ( nPCreserve );
        m_sPC.prev()->colors_.reserve ( nPCreserve );
        m_sPC.next()->points_.reserve ( nPCreserve );
        m_sPC.next()->colors_.reserve ( nPCreserve );
    }
    
    //ring buf
    pK->v ( "nP", &m_nP );
    if(m_nP > 0)
    {
        m_pP = new PC_POINT[m_nP];
        NULL_F(m_pP);
        m_iP = 0;
        m_tP = 0;
    }

    //transform
    pK->v("bTransform", &m_bTransform);
    pK->v("vT", &m_vT);
	pK->v("vR", &m_vR);

    string n;

    n = "";
    pK->v ( "_PCbase", &n );
    m_pPCB = ( _PCbase* ) ( pK->getInst ( n ) );

    n = "";
    pK->v ( "_PCviewer", &n );
    m_pViewer = ( _PCviewer* ) ( pK->getInst ( n ) );

    if ( m_pViewer )
    {
        m_iV = m_pViewer->addGeometry();
    }

    return true;
}

int _PCbase::check ( void )
{
    NULL__(m_pP, -1);
    
    return this->_ModuleBase::check();
}

void _PCbase::getPC ( PointCloud* pPC )
{
    NULL_ ( pPC );

    pthread_mutex_lock ( &m_mutexPC );
    *pPC = *m_sPC.prev();
    pthread_mutex_unlock ( &m_mutexPC );
}

void _PCbase::updatePC ( void )
{
    paintPC(m_sPC.next());
    
    pthread_mutex_lock ( &m_mutexPC );
    m_sPC.update();
    m_sPC.next()->points_.clear();
    m_sPC.next()->colors_.clear();
    m_sPC.next()->normals_.clear();
    pthread_mutex_unlock ( &m_mutexPC );
}

void _PCbase::paintPC ( PointCloud* pPC )
{
    NULL_ ( pPC );
    IF_ ( m_vColOvrr.x < 0.0 )

    pPC->PaintUniformColor (
        Eigen::Vector3d (
            m_vColOvrr.x,
            m_vColOvrr.y,
            m_vColOvrr.z
        )
    );
}

int _PCbase::size ( void )
{
    return m_sPC.prev()->points_.size();
}

void _PCbase::addP(Eigen::Vector3d& vP, Eigen::Vector3d& vC, uint64_t& tStamp)
{
    IF_(check() < 0);
    
    if(++m_iP >= m_nP)
        m_iP = 0;
    
    PC_POINT* pP = &m_pP[m_iP];
    pP->m_vP = m_A * vP;
    pP->m_vC = vC;
    pP->m_tStamp = tStamp;
}

void _PCbase::updateTransformMatrix(void)
{
	Eigen::Matrix4d mT;
	Eigen::Vector3d vR(m_vR.x, m_vR.y, m_vR.z);
    mT.block(0,0,3,3) = Geometry3D::GetRotationMatrixFromXYZ(vR);
	mT(0,3) = m_vT.x;
	mT(1,3) = m_vT.y;
	mT(2,3) = m_vT.z;

    m_A = mT;
}

void _PCbase::setTranslation(vDouble3& vT)
{
	m_vT = vT;
}

vDouble3 _PCbase::getTranslation(void)
{
	return m_vT;
}

void _PCbase::setRotation(vDouble3& vR)
{
	m_vR = vR;
}

vDouble3 _PCbase::getRotation(void)
{
	return m_vR;
}

void _PCbase::draw ( void )
{
    this->_ModuleBase::draw();
}

}
#endif
