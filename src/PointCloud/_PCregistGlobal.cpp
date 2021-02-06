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
    m_maxNN = 30;

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
    pK->v ( "maxNN", &m_maxNN );

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
    NULL_l(m_pSource, -1);
    NULL_l(m_pTarget, -1);
    NULL_l(m_pTf, -1);
    
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

void _PCregistGlobal::preprocess(PointCloud& pc)
{
    double rNormal = m_voxelSize * 2;
    pc.EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(rNormal, m_maxNN));
    
    double rFeature = m_voxelSize * 5;
    std::shared_ptr<open3d::pipelines::registration::Feature> spF =
    open3d::pipelines::registration::ComputeFPFHFeature(
        pc,
        open3d::geometry::KDTreeSearchParamHybrid(rFeature, m_maxNN)
    );
    
//    return spF;

/*    radius_normal = voxel_size * 2
    print(":: Estimate normal with search radius %.3f." % radius_normal)
    pcd_down.estimate_normals(
        o3d.geometry.KDTreeSearchParamHybrid(radius=radius_normal, max_nn=30))

    radius_feature = voxel_size * 5
    print(":: Compute FPFH feature with search radius %.3f." % radius_feature)
    pcd_fpfh = o3d.pipelines.registration.compute_fpfh_feature(
        pcd_down,
        o3d.geometry.KDTreeSearchParamHybrid(radius=radius_feature, max_nn=100))
    return pcd_down, pcd_fpfh
*/    
}

void _PCregistGlobal::updateRegistration ( void )
{
    IF_ ( check() < 0 );

    PointCloud pcSource;
    m_pSource->getPC ( &pcSource );
    PointCloud pcTarget;
    m_pTarget->getPC ( &pcTarget );

    double rNormal = pcSourve
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
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

void _PCregistGlobal::draw ( void )
{
    this->_PCbase::draw();
}

}
#endif
