#include "_AP_droneBoxJSON.h"

namespace kai
{

_AP_droneBoxJSON::_AP_droneBoxJSON()
{
    m_pAPgs = NULL;
}

_AP_droneBoxJSON::~_AP_droneBoxJSON()
{
}

bool _AP_droneBoxJSON::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;
    
    int v = USEC_1SEC;
    pK->v("tIntHeartbeat", &v);
    m_tIntHeartbeat.init(v);
    
    string n;
    n = "";
    pK->v ( "_AP_gs", &n );
    m_pAPgs = ( _AP_gs* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pAPgs, n + ": not found" );
    return true;
}

bool _AP_droneBoxJSON::start ( void )
{
    int retCode;

    if ( !m_bThreadON )
    {
        m_bThreadON = true;
        retCode = pthread_create ( &m_threadID, 0, getUpdateThreadW, this );
        if ( retCode != 0 )
        {
            LOG_E ( retCode );
            m_bThreadON = false;
            return false;
        }
    }

    if ( !m_bRThreadON )
    {
        m_bRThreadON = true;
        retCode = pthread_create ( &m_rThreadID, 0, getUpdateThreadR, this );
        if ( retCode != 0 )
        {
            LOG_E ( retCode );
            m_bRThreadON = false;
            return false;
        }
    }

    return true;
}

int _AP_droneBoxJSON::check ( void )
{
    NULL__ ( m_pAPgs, -1 );

    return this->_JSONbase::check();
}

void _AP_droneBoxJSON::updateW ( void )
{
    while ( m_bThreadON )
    {
        if ( !m_pIO )
        {
            this->sleepTime ( USEC_1SEC );
            continue;
        }

        if ( !m_pIO->isOpen() )
        {
            if ( !m_pIO->open() )
            {
                this->sleepTime ( USEC_1SEC );
                continue;
            }
        }

        this->autoFPSfrom();

        send();

        this->autoFPSto();
    }
}

void _AP_droneBoxJSON::send ( void )
{
    IF_ ( check() <0 );

    this->_JSONbase::send();
}

void _AP_droneBoxJSON::updateR ( void )
{
    while ( m_bRThreadON )
    {
        if(recv())
        {
            handleMsg(m_strB);
            m_strB.clear();            
        }
        this->sleepTime ( 0 ); //wait for the IObase to wake me up when received data
    }
}

void _AP_droneBoxJSON::handleMsg ( string& str )
{
    value json;    
    IF_(!str2JSON(str,&json));
    
    object& jo = json.get<object>();
    string cmd = jo["cmd"].get<string>();

    if ( cmd == "heartbeat" )
        heartbeat(jo);
    else if ( cmd == "ackLandingRequest" )
        ackLandingRequest(jo);
    else if ( cmd == "ackTakeoffRequest" )
        ackTakeoffRequest(jo);
}

void _AP_droneBoxJSON::heartbeat(picojson::object& o)
{
    IF_(check()<0 );
    
}

void _AP_droneBoxJSON::ackLandingRequest (picojson::object& o)
{
    IF_(check()<0 );

}

void _AP_droneBoxJSON::ackTakeoffRequest (picojson::object& o)
{
    IF_(check()<0 );
    
}

void _AP_droneBoxJSON::draw ( void )
{
    this->_JSONbase::draw();

    string msg;
    addMsg ( msg );
}

}
