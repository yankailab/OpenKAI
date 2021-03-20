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
    
    Eigen::Vector3d(-1.0, -1.0, -1.0);
}

_Livox::~_Livox()
{
}

bool _Livox::init ( void* pKiss )
{
    IF_F ( !this->_PCstream::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "broadcastCode", &m_broadcastCode );

    string n;
    n = "";
    pK->v ( "LivoxLidar", &n );
    m_pL = ( LivoxLidar* ) ( pK->getInst ( n ) );

    return true;
}

bool _Livox::open ( void )
{
    IF_F ( check() <0 );

    IF_F ( !m_pL->setDataCallback ( m_broadcastCode, CbRecvData, ( void* ) this ) );

    m_bOpen = true;

    LOG_I ( "Init LivoxLidar success! Starting discovering Lidars\n" );
    return true;
}

void _Livox::close ( void )
{
}

int _Livox::check ( void )
{
    NULL__ ( m_pL, -1 );

    return this->_PCstream::check();
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

        m_pT->autoFPSto();
    }
}

bool _Livox::updateLidar ( void )
{

    return true;
}

void _Livox::CbRecvData ( LivoxEthPacket* pData, void* pLivox )
{
    NULL_ ( pData );
    NULL_ ( pLivox );

    uint64_t tStamp = * ( ( uint64_t * ) ( pData->timestamp ) );
    _Livox* pL = ( _Livox* ) pLivox;

    if ( pData ->data_type == kCartesian )
    {
        LivoxRawPoint *p_point_data = ( LivoxRawPoint * ) pData->data;
    }
    else if ( pData ->data_type == kExtendCartesian )
    {
        pL->addP ( ( LivoxExtendRawPoint* ) pData->data, tStamp );
    }
    else if ( pData ->data_type == kDualExtendCartesian )
    {
        pL->addDualP ( ( LivoxDualExtendRawPoint* ) pData->data, tStamp );
    }
    else if ( pData ->data_type == kTripleExtendCartesian )
    {
        pL->addTripleP ( ( LivoxTripleExtendRawPoint* ) pData->data, tStamp );
    }
    else if ( pData ->data_type == kImu )
    {
        pL->updateIMU ( ( LivoxImuPoint* ) pData->data );
    }
    
}

void _Livox::addP ( LivoxExtendRawPoint* pLp, uint64_t& tStamp )
{
    LivoxExtendRawPoint* pP = ( LivoxExtendRawPoint * ) pLp;
    Eigen::Vector3d vP(pP->x, pP->y, pP->z);
    this->_PCstream::add(vP, m_vCol, tStamp);
}

void _Livox::addDualP ( LivoxDualExtendRawPoint* pLp, uint64_t& tStamp )
{
    LivoxDualExtendRawPoint* pP = ( LivoxDualExtendRawPoint *) pLp;

    Eigen::Vector3d vP1(pP->x1, pP->y1, pP->z1);
    Eigen::Vector3d vP2(pP->x2, pP->y2, pP->z2);

    this->_PCstream::add(vP1, m_vCol, tStamp);
    this->_PCstream::add(vP2, m_vCol, tStamp);
}

void _Livox::addTripleP ( LivoxTripleExtendRawPoint* pLp, uint64_t& tStamp )
{
    LivoxTripleExtendRawPoint* pP = ( LivoxTripleExtendRawPoint *) pLp;

    Eigen::Vector3d vP1(pP->x1, pP->y1, pP->z1);
    Eigen::Vector3d vP2(pP->x2, pP->y2, pP->z2);
    Eigen::Vector3d vP3(pP->x3, pP->y3, pP->z3);

    this->_PCstream::add(vP1, m_vCol, tStamp);
    this->_PCstream::add(vP2, m_vCol, tStamp);
    this->_PCstream::add(vP3, m_vCol, tStamp);
}

void _Livox::updateIMU ( LivoxImuPoint* pLd )
{
    LivoxImuPoint *pD = ( LivoxImuPoint * ) pLd;

    printf ( "gxyz=(%f, %f, %f) axyz=(%f, %f, %f)\n",
             pD->gyro_x,
             pD->gyro_y,
             pD->gyro_z,
             pD->acc_x,
             pD->acc_y,
             pD->acc_z );
}

void _Livox::draw ( void )
{
    this->_PCstream::draw();

}

}
#endif

