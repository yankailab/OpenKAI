#include "_AP_droneBoxJSON.h"

namespace kai
{

_AP_droneBoxJSON::_AP_droneBoxJSON()
{
    m_Tr = NULL;
    m_pAPgcs = NULL;
}

_AP_droneBoxJSON::~_AP_droneBoxJSON()
{
    DEL(m_pTr);
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
    
    Kiss* pKt = pK->child("threadR");
    IF_F(pKt->empty());
    
    m_pTr = new _Thread();
    if(!m_pTr->init(pKt))
    {
        DEL(m_pTr);
        return false;
    }
    
    return true;
}

bool _AP_droneBoxJSON::start ( void )
{
    NULL_F(m_pT);
    NULL_F(m_pTr);
	IF_F(!m_pT->start(getUpdateW, this));
	return m_pTr->start(getUpdateR, this);
}

int _AP_droneBoxJSON::check ( void )
{
    NULL__ ( m_pAPgcs, -1 );

    return this->_JSONbase::check();
}

void _AP_droneBoxJSON::updateW ( void )
{
    while(m_pT->bRun())
    {
        if ( !m_pIO )
        {
            m_pT->sleepT ( USEC_1SEC );
            continue;
        }

        if ( !m_pIO->isOpen() )
        {
            if ( !m_pIO->open() )
            {
                m_pT->sleepT ( USEC_1SEC );
                continue;
            }
        }

        m_pT->autoFPSfrom();

        send();

        m_pT->autoFPSto();
    }
}

void _AP_droneBoxJSON::send ( void )
{
    IF_ ( check() <0 );

    this->_JSONbase::send();

    GCS_STATE* pState = m_pAPgcs->getState();
        
    object o;
    JO(o, "id", (double)1);
    JO(o, "t", (double)m_pT->getTstamp());

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
    while ( m_pTr->bRun() )
    {
        if(recv())
        {
            handleMsg(m_strB);
            m_strB.clear();            
        }
        
        if(m_strB.empty())
            m_pT->sleepT ( 100000 ); //wait for the IObase to wake me up when received data
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
