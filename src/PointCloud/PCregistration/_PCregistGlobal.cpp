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
    m_rNormal = 0.2;
    m_rFeature = 0.5;
    m_maxNNnormal = 30;
    m_maxNNfpfh = 100;
    m_lastFit = 0.0;

    m_pSrc = NULL;
    m_pTgt = NULL;
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

    pK->v ( "iMt", &m_iMt );
    pK->v ( "rNormal", &m_rNormal );
    pK->v ( "rFeature", &m_rFeature );
    pK->v ( "maxNNnormal", &m_maxNNnormal );
    pK->v ( "maxNNfpfh", &m_maxNNfpfh );

    string n;

    n = "";
    pK->v ( "_PCbaseSrc", &n );
    m_pSrc = ( _PCbase* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pSrc, n + ": not found" );

    n = "";
    pK->v ( "_PCbaseTgt", &n );
    m_pTgt = ( _PCbase* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pTgt, n + ": not found" );

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
    NULL__( m_pSrc, -1);
    NULL__( m_pTgt, -1);
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

    IF_(!fastGlobalRegistration());
    
    m_pTf->setTranslationMatrix ( m_iMt, m_RR.transformation_ );    
}

bool _PCregistGlobal::fastGlobalRegistration(void)
{
    PointCloud pcSrc;
    m_pSrc->getPC ( &pcSrc );
    PointCloud pcTgt;
    m_pTgt->getPC ( &pcTgt );
    
    IF_F(pcSrc.IsEmpty());
    IF_F(pcTgt.IsEmpty());
    
    Feature spFpfhSrc = *preprocess( pcSrc );
    Feature spFpfhTgt = *preprocess( pcTgt );

    m_RR = FastGlobalRegistration
            (
                pcSrc,
                pcTgt,
                spFpfhSrc,
                spFpfhTgt,
                FastGlobalRegistrationOption()
            );
    
    IF_F(m_RR.fitness_ < m_lastFit);
    m_lastFit = m_RR.fitness_;
    
    return true;
}

std::shared_ptr<Feature> _PCregistGlobal::preprocess(PointCloud& pc)
{
    pc.EstimateNormals(KDTreeSearchParamHybrid(m_rNormal, m_maxNNnormal ));
    return ComputeFPFHFeature
    (
        pc,
        KDTreeSearchParamHybrid(m_rFeature, m_maxNNfpfh )
    );    
}

void _PCregistGlobal::draw ( void )
{
    this->_PCbase::draw();
    addMsg("Fitness = " + f2str((float)m_RR.fitness_) + 
            ", Inliner_rmse = " + f2str((float)m_RR.inlier_rmse_));    
}

}
#endif
