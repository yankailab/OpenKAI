#include "_DepthCam.h"

namespace kai
{

_DepthCam::_DepthCam()
{
    m_Tr = NULL;
    m_pDB = NULL;
}

_DepthCam::~_DepthCam()
{
    DEL ( m_pTr );
}

bool _DepthCam::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    string n;
    n = "";
    pK->v ( "_DepthVisionBase", &n );
    m_pDB = ( _DepthVisionBase* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pDB, n + ": not found" );

    Kiss* pKt = pK->child ( "threadR" );
    IF_F ( pKt->empty() );

    m_pTr = new _Thread();
    if ( !m_pTr->init ( pKt ) )
    {
        DEL ( m_pTr );
        return false;
    }

    return true;
}

bool _DepthCam::start ( void )
{
    NULL_F ( m_pT );
    NULL_F ( m_pTr );
    IF_F ( !m_pT->start ( getUpdateW, this ) );
    return m_pTr->start ( getUpdateR, this );
}

int _DepthCam::check ( void )
{
    NULL__ ( m_pDB, -1 );

    return this->_JSONbase::check();
}

void _DepthCam::updateW ( void )
{
    while ( m_pT->bRun() )
    {
        if ( !m_pIO )
        {
            m_pT->sleepT ( SEC_2_USEC );
            continue;
        }

        if ( !m_pIO->isOpen() )
        {
            if ( !m_pIO->open() )
            {
                m_pT->sleepT ( SEC_2_USEC );
                continue;
            }
        }

        m_pT->autoFPSfrom();

        send();

        m_pT->autoFPSto();
    }
}

void _DepthCam::send ( void )
{
    IF_ ( check() <0 );

    this->_JSONbase::send();
}

void _DepthCam::updateR ( void )
{
    while ( m_pTr->bRun() )
    {
        m_pTr->autoFPSfrom();

        if ( recv() )
        {
            handleMsg ( m_strB );
            m_strB.clear();
        }

        m_pTr->autoFPSto();
    }
}

void _DepthCam::handleMsg ( string& str )
{
    value json;
    IF_ ( !str2JSON ( str,&json ) );

    object& jo = json.get<object>();
    IF_ ( !jo["cmd"].is<string>() );
    string cmd = jo["cmd"].get<string>();

    if ( cmd == "heartbeat" )
        heartbeat ( jo );
    else if ( cmd == "stat" )
        stat ( jo );
    else if ( cmd == "req" )
        req ( jo );
}

void _DepthCam::heartbeat ( picojson::object& o )
{
    IF_ ( check() <0 );

}

void _DepthCam::stat ( picojson::object& o )
{
    IF_ ( check() <0 );
    IF_ ( !o["id"].is<double>() );
    IF_ ( !o["stat"].is<string>() );

    int vID = o["id"].get<double>();
    string stat = o["stat"].get<string>();

}

void _DepthCam::req ( picojson::object& o )
{
    IF_ ( check() <0 );
    IF_ ( !o["id"].is<double>() );
    IF_ ( !o["do"].is<string>() );

    int vID = o["id"].get<double>();
    string d = o["do"].get<string>();

    object jo;
    JO ( jo, "cmd", "ack" );
    JO ( jo, "do", d );

    sendMsg ( jo );
}

void _DepthCam::console ( void* pConsole )
{
    NULL_(pConsole);
    this->_JSONbase::console(pConsole);
    
    NULL_(m_pTr);
    m_pTr->console(pConsole);
}

}
