#include "_AP_droneBoxJSON.h"

namespace kai
{

_AP_droneBoxJSON::_AP_droneBoxJSON()
{
    m_pAP = NULL;
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
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pAP, n + ": not found" );

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
    NULL__ ( m_pAP, -1 );

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

    if(m_tIntHeartbeat.update(m_tStamp))
    {
        sendHeartbeat();
    }
}

void _AP_droneBoxJSON::sendHeartbeat (void)
{
    object o;
    JO(o, "id", "tf" + i2str(1));

    string msg = picojson::value ( o ).serialize() + m_msgFinishSend;
    m_pIO->write ( ( unsigned char* ) msg.c_str(), msg.size() );
}

bool _AP_droneBoxJSON::sendMsg (picojson::object& o)
{
    string msg = picojson::value ( o ).serialize() + m_msgFinishSend;
    return m_pIO->write ( ( unsigned char* ) msg.c_str(), msg.size() );  
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
    string err;
    const char* jsonstr = str.c_str();
    value json;
    parse ( json, jsonstr, jsonstr + strlen ( jsonstr ), &err );
    IF_ ( !json.is<object>() );

    object& jo = json.get<object>();
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

void _AP_droneBoxJSON::heartbeat(picojson::object& o)
{
    IF_(check()<0 );
    
}

void _AP_droneBoxJSON::landingRequest (picojson::object& o)
{
    IF_(check()<0 );
    
//    m_pDB->landingRequest(o["vID"].get<int>());
}

void _AP_droneBoxJSON::landingStatus (picojson::object& o)
{
    IF_(check()<0 );

}

void _AP_droneBoxJSON::takeoffRequest (picojson::object& o)
{
    IF_(check()<0 );
    
}

void _AP_droneBoxJSON::takeoffStatus (picojson::object& o)
{
    IF_(check()<0 );
    
}

void _AP_droneBoxJSON::draw ( void )
{
    this->_ThreadBase::draw();

    string msg;
    if ( m_pIO->isOpen() )
        msg = "STANDBY, CONNECTED";
    else
        msg = "STANDBY, Not connected";

    addMsg ( msg );

}

}
