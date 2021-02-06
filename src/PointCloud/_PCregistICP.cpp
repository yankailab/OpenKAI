/*
 * _PCregistICP.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#include "_PCregistICP.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCregistICP::_PCregistICP()
{
    m_thr = 0.02;
    m_nMinP = 1000;

    m_pSource = NULL;
    m_pTarget = NULL;
    m_pTf = NULL;
    m_iMt = 0;
}

_PCregistICP::~_PCregistICP()
{
}

bool _PCregistICP::init ( void *pKiss )
{
    IF_F ( !_PCbase::init ( pKiss ) );
    Kiss *pK = ( Kiss* ) pKiss;

    pK->v ( "thr", &m_thr );
    pK->v ( "nMinP", &m_nMinP );
    pK->v ( "iMt", &P.m_iMt );

    string n;

    n = "";
    pK->v ( "source", &n );
    m_pSource = ( _PCbase* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pSource, n + ": not found" );

    n = "";
    pK->v ( "target", &n );
    m_pTarget = ( _PCbase* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pTarget, n + ": not found" );

    n = "";
    pK->v ( "_PCtransform", &n );
    m_pTf = ( _PCtransform* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pTf, n + ": not found" );

    return true;
}

bool _PCregistICP::start ( void )
{
    NULL_F ( m_pT );
    return m_pT->start ( getUpdate, this );
}

int _PCregistICP::check ( void )
{
    NULL_l(m_pSource, -1);
    NULL_l(m_pTarget, -1);
    NULL_l(m_pTf, -1);

    return _PCbase::check();
}

void _PCregistICP::update ( void )
{
    while ( m_pT->bRun() )
    {
        m_pT->autoFPSfrom();

        updateRegistration();

        m_pT->autoFPSto();
    }
}

void _PCregistICP::updateRegistration ( void )
{
    IF_ ( check() < 0 );

    PointCloud pcSource;
    m_pSource->getPC ( &pcSource );
    PointCloud pcTarget;
    m_pTarget->getPC ( &pcTarget );

    pipelines::registration::RegistrationResult rr = pipelines::registration::RegistrationICP (
                pcSource,
                pcTarget,
                m_thr,
                Eigen::Matrix4d::Identity(),
                pipelines::registration::TransformationEstimationPointToPoint()
            );

    Eigen::Matrix4d_u m = m_pTf->getTranslationMatrix ( p->m_iMt ) * rr.transformation_;
    m_pTf->setTranslationMatrix ( p->m_iMt, m );
}

void _PCregistICP::draw ( void )
{
    this->_PCbase::draw();
}

}
#endif
