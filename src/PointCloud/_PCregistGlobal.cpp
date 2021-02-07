/*
 * _PCregistGlobal.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#include "_PCregistGlobal.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCregistGlobal::_PCregistGlobal()
{
    m_thr = 0.02;
    m_nMinP = 1000;
    
    m_voxelSize = 0.1;
    m_maxNNnormal = 30;
    m_maxNNfpfh = 100;

    m_pSource = NULL;
    m_pTarget = NULL;
    m_pTf = NULL;
    m_iMt = 0;
}

_PCregistGlobal::~_PCregistGlobal()
{
}

bool _PCregistGlobal::init ( void *pKiss )
{
    IF_F ( !_PCbase::init ( pKiss ) );
    Kiss *pK = ( Kiss* ) pKiss;

    pK->v ( "thr", &m_thr );
    pK->v ( "nMinP", &m_nMinP );
    pK->v ( "iMt", &m_iMt );
    pK->v ( "voxelSize", &m_voxelSize );
    pK->v ( "maxNNnormal", &m_maxNNnormal );
    pK->v ( "maxNNfpfh", &m_maxNNnormal );

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

bool _PCregistGlobal::start ( void )
{
    NULL_F ( m_pT );
    return m_pT->start ( getUpdate, this );
}

int _PCregistGlobal::check ( void )
{
    NULL__(m_pSource, -1);
    NULL__(m_pTarget, -1);
    NULL__(m_pTf, -1);
    
    return _PCbase::check();
}

void _PCregistGlobal::update ( void )
{
    while ( m_pT->bRun() )
    {
        m_pT->autoFPSfrom();

        updateRegistration();

        m_pT->autoFPSto();
    }
}

void _PCregistGlobal::updateRegistration ( void )
{
    IF_ ( check() < 0 );

    m_RR = fastGlobalRegistration();

}

RegistrationResult _PCregistGlobal::fastGlobalRegistration(void)
{
    PointCloud pcSource;
    m_pSource->getPC ( &pcSource );
    PointCloud pcTarget;
    m_pTarget->getPC ( &pcTarget );
    
    Feature spFpfhSrc = *preprocess(pcSource);
    Feature spFpfhTgt = *preprocess(pcTarget);

    double dThr = m_voxelSize * 0.5;
    
    return
    FastGlobalRegistration
    (
        pcSource,
        pcTarget,
        spFpfhSrc,
        spFpfhTgt,
        open3d::pipelines::registration::FastGlobalRegistrationOption()
    );
}

std::shared_ptr<Feature> _PCregistGlobal::preprocess(PointCloud& pc)
{
    double rNormal = m_voxelSize * 2;
    pc.EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(rNormal, m_maxNNnormal ));
    
    double rFeature = m_voxelSize * 5;
    return open3d::pipelines::registration::ComputeFPFHFeature
    (
        pc,
        open3d::geometry::KDTreeSearchParamHybrid(rFeature, m_maxNNfpfh )
    );    
}

void _PCregistGlobal::draw ( void )
{
    this->_PCbase::draw();
}

}
#endif
