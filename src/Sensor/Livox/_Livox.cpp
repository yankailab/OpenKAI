/*
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */
#include "_Livox.h"

#ifdef USE_LIVOX

namespace kai
{

_Livox::_Livox()
{
    m_bOpen = false;
    m_pL = NULL;
    m_iTransformed = 0;
}

_Livox::~_Livox()
{
}

bool _Livox::init ( void* pKiss )
{
    IF_F ( !this->_PCbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "broadcastCode", &m_broadcastCode );

    string n;
    n = "";
    pK->v ( "lds_lidar", &n );
    m_pL = ( LdsLidar* ) ( pK->getInst ( n ) );

    return true;
}

bool _Livox::open ( void )
{
    IF_F ( check() <0 );

    IF_F ( !m_pL->setDataCallback ( m_broadcastCode, CbRecvData, ( void* ) this ) );
    
    m_bOpen = true;

    LOG_I ( "Init lds lidar success! Starting discovering Lidars\n" );
    return true;
}

void _Livox::close ( void )
{
}

int _Livox::check ( void )
{
    NULL__ ( m_pL, -1 );

    return this->_PCbase::check();
}

bool _Livox::start ( void )
{
    NULL_F ( m_pT );
    return m_pT->start ( getUpdate, this );
}

void _Livox::update ( void )
{
    while ( m_pT->bRun() )
    {
        if ( !m_bOpen )
        {
            if ( !open() )
            {
                m_pT->sleepT ( USEC_1SEC );
                continue;
            }
        }

        m_pT->autoFPSfrom();

        updateLidar();

        if ( m_pViewer )
        {
            m_pViewer->updateGeometry ( m_iV, m_sPC.prev() );
        }

        m_pT->autoFPSto();
    }
}

bool _Livox::updateLidar ( void )
{
    PointCloud* pPC = m_sPC.next();

    if ( m_bTransform )
    {
        while ( m_iTransformed < pPC->points_.size() )
        {
            Eigen::Vector3d* pP = &pPC->points_[m_iTransformed];
            *pP = m_A * (*pP);
            
            m_iTransformed++;
        }
    }

    return true;
}

void _Livox::CbRecvData ( LivoxEthPacket* pData, void* pLivox )
{
    NULL_ ( pData );
    NULL_ ( pLivox );

    _Livox* pL = ( _Livox* ) pLivox;

    // Parsing the timestamp and the point cloud data.
    uint64_t cur_timestamp = * ( ( uint64_t * ) ( pData->timestamp ) );
    if ( pData ->data_type == kCartesian )
    {
        LivoxRawPoint *p_point_data = ( LivoxRawPoint * ) pData->data;
    }
    else if ( pData ->data_type == kSpherical )
    {
        LivoxSpherPoint *p_point_data = ( LivoxSpherPoint * ) pData->data;
    }
    else if ( pData ->data_type == kExtendCartesian )
    {
        LivoxExtendRawPoint *pD = ( LivoxExtendRawPoint * ) pData->data;
        
        printf("xyzrt=(%d, %d, %d, %d, %d)\n", pD->x, pD->y, pD->z, pD->reflectivity, pD->tag);
    }
    else if ( pData ->data_type == kExtendSpherical )
    {
        LivoxExtendSpherPoint *p_point_data = ( LivoxExtendSpherPoint * ) pData->data;
    }
    else if ( pData ->data_type == kDualExtendCartesian )
    {
        LivoxDualExtendRawPoint *p_point_data = ( LivoxDualExtendRawPoint * ) pData->data;
    }
    else if ( pData ->data_type == kDualExtendSpherical )
    {
        LivoxDualExtendSpherPoint *p_point_data = ( LivoxDualExtendSpherPoint * ) pData->data;
    }
    else if ( pData ->data_type == kImu )
    {
        LivoxImuPoint *p_point_data = ( LivoxImuPoint * ) pData->data;
    }
    else if ( pData ->data_type == kTripleExtendCartesian )
    {
        LivoxTripleExtendRawPoint *p_point_data = ( LivoxTripleExtendRawPoint * ) pData->data;
    }
    else if ( pData ->data_type == kTripleExtendSpherical )
    {
        LivoxTripleExtendSpherPoint *p_point_data = ( LivoxTripleExtendSpherPoint * ) pData->data;
    }

}

void _Livox::draw ( void )
{
    this->_PCbase::draw();

}

}
#endif

