#include "_AP_droneBoxJSON.h"

namespace kai
{

_AP_droneBoxJSON::_AP_droneBoxJSON()
{
    m_pAPgcs = NULL;
}

_AP_droneBoxJSON::~_AP_droneBoxJSON()
{
}

bool _AP_droneBoxJSON::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    string n;
    n = "";
    pK->v ( "_AP_gcs", &n );
    m_pAPgcs = ( _AP_gcs* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pAPgcs, n + ": not found" );
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
    NULL__ ( m_pAPgcs, -1 );

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

    GCS_STATE* pState = m_pAPgcs->getState();
        
    object o;
    JO(o, "id", (double)1);
    JO(o, "t", (double)m_tStamp);

    if(pState->bTAKEOFF_REQUEST())
    {
        JO(o, "cmd", "takeoffRequest");
        sendMsg(o);
    }
    else if(pState->bAIRBORNE())
    {
        JO(o, "cmd", "takeoffStatus");
        JO(o, "stat", "airborne");
        sendMsg(o);
    }
    else if(pState->bLANDING_REQUEST())
    {
        JO(o, "cmd", "landingRequest");
        sendMsg(o);       
    }
    else if(pState->bSTANDBY())
    {
        JO(o, "cmd", "landingStatus");
        JO(o, "stat", "standby");
        sendMsg(o);       
    }
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

    IF_(!jo["cmd"].is<string>());
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
    IF_(!o["id"].is<double>());
    IF_(!o["result"].is<string>());
        
    string r = o["result"].get<string>();
    bool bReady = ( r == "ok");

    m_pAPgcs->landingReady(bReady);
}

void _AP_droneBoxJSON::ackTakeoffRequest (picojson::object& o)
{
    IF_(check()<0 );
    IF_(!o["id"].is<double>());
    IF_(!o["result"].is<string>());
    
    string r = o["result"].get<string>();
    bool bReady = ( r == "ok");

    m_pAPgcs->takeoffReady(bReady);
}

void _AP_droneBoxJSON::draw ( void )
{
    this->_JSONbase::draw();
}

}
