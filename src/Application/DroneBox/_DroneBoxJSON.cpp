#include "_DroneBoxJSON.h"

namespace kai
{

_DroneBoxJSON::_DroneBoxJSON()
{
    m_Tr = NULL;
    m_pDB = NULL;
}

_DroneBoxJSON::~_DroneBoxJSON()
{
    DEL(m_pTr);
}

bool _DroneBoxJSON::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;
        
    string n;
    n = "";
    pK->v ( "_DroneBox", &n );
    m_pDB = ( _DroneBox* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pDB, n + ": not found" );
    
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

bool _DroneBoxJSON::start ( void )
{
    NULL_F(m_pT);
    NULL_F(m_pTr);
	IF_F(!m_pT->start(getUpdateW, this));
	return m_pTr->start(getUpdateR, this);
}

int _DroneBoxJSON::check ( void )
{
    NULL__ ( m_pDB, -1 );

    return this->_JSONbase::check();
}

void _DroneBoxJSON::updateW ( void )
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

void _DroneBoxJSON::send ( void )
{
    IF_ ( check() <0 );

    this->_JSONbase::send();
}

void _DroneBoxJSON::updateR ( void )
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

void _DroneBoxJSON::handleMsg ( string& str )
{
    value json;
    IF_(!str2JSON(str,&json));
    
    object& jo = json.get<object>();
    
    IF_(!jo["cmd"].is<string>());
    string cmd = jo["cmd"].get<string>();

    if ( cmd == "heartbeat" )
        heartbeat(jo);
    else if ( cmd == "landingRequest" )
        landingRequest(jo);
    else if ( cmd == "landingStatus" )
        landingStatus(jo);
    else if ( cmd == "takeoffRequest" )
        takeoffRequest(jo);
    else if ( cmd == "takeoffStatus" )
        takeoffStatus(jo);
}

void _DroneBoxJSON::heartbeat(picojson::object& o)
{
    IF_(check()<0 );
    
}

void _DroneBoxJSON::landingRequest (picojson::object& o)
{
    IF_(check()<0 );
    IF_(!o["id"].is<double>());
    
    int vID = o["id"].get<double>();
    object jo;
    JO(jo, "id", i2str(m_pDB->getID()));
    JO(jo, "cmd", "ackLandingRequest");
    
    if(m_pDB->landingRequest(vID))
    {
        JO(jo, "result", "ok");        
    }
    else
    {
        JO(jo, "result", "denied");        
    }
    
    sendMsg(jo);
}

void _DroneBoxJSON::landingStatus (picojson::object& o)
{
    IF_(check()<0 );
    IF_(!o["id"].is<double>());
    IF_(!o["stat"].is<string>());
    
    int vID = o["id"].get<double>();
    string stat = o["stat"].get<string>();
    bool bComplete = (stat == "standby");

    m_pDB->landingStatus(vID, bComplete);
}

void _DroneBoxJSON::takeoffRequest (picojson::object& o)
{
    IF_(check()<0);
    IF_(!o["id"].is<double>());
    
    int vID = o["id"].get<double>();
    object jo;
    JO(jo, "id", i2str(m_pDB->getID()));
    JO(jo, "cmd", "ackTakeoffRequest");
    
    if(m_pDB->takeoffRequest(vID))
    {
        JO(jo, "result", "ok");        
    }
    else
    {
        JO(jo, "result", "denied");        
    }
    
    sendMsg(jo);
}

void _DroneBoxJSON::takeoffStatus (picojson::object& o)
{
    IF_(check()<0 );
    IF_(!o["id"].is<double>());
    IF_(!o["stat"].is<string>());
    
    int vID = o["id"].get<double>();
    string stat = o["stat"].get<string>();
    bool bComplete = (stat == "airborne");
    
    m_pDB->takeoffStatus(vID, bComplete);
}

void _DroneBoxJSON::draw ( void )
{
    this->_JSONbase::draw();
}

}
