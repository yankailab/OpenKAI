#include "_DroneBoxJSON.h"

namespace kai
{

_DroneBoxJSON::_DroneBoxJSON()
{
}

_DroneBoxJSON::~_DroneBoxJSON()
{
}

bool _DroneBoxJSON::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;
    
    int v = USEC_1SEC;
    pK->v("tIntHeartbeat", &v);
    m_tIntHeartbeat.init(v);

    return true;
}

bool _DroneBoxJSON::start ( void )
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

int _DroneBoxJSON::check ( void )
{
    return this->_JSONbase::check();
}

void _DroneBoxJSON::updateW ( void )
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

void _DroneBoxJSON::send ( void )
{
    IF_ ( check() <0 );

    
}

void _DroneBoxJSON::heartbeat(void)
{
    object o;
    o.insert ( make_pair ( "id", value ( "tf" + i2str ( 1 ) ) ) );
    o.insert ( make_pair ( "state", value ( "NORMAL" ) ) );

    string msg = picojson::value ( o ).serialize() + m_msgFinishSend;
    m_pIO->write ( ( unsigned char* ) msg.c_str(), msg.size() );
}

void _DroneBoxJSON::actionReuqest(void)
{
    
}

void _DroneBoxJSON::actionComplete(void)
{
    
}

void _DroneBoxJSON::actionReply(void)
{
    
}

void _DroneBoxJSON::updateR ( void )
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

void _DroneBoxJSON::handleMsg ( string& str )
{
    string err;
    const char* jsonstr = str.c_str();
    value json;
    parse ( json, jsonstr, jsonstr + strlen ( jsonstr ), &err );
    IF_ ( !json.is<object>() );

    object& jo = json.get<object>();
    string cmd = jo["cmd"].get<string>();

    if ( cmd == "heartbeat" )
    {
    }
    else if ( cmd == "actionRequest" )
    {
    }
    else if ( cmd == "actionComplete" )
    {
    }
    else if ( cmd == "actionReply" )
    {
    }
    else if ( cmd == "actionResult" )
    {
    }

}

void _DroneBoxJSON::heartbeat(picojson::object& jo)
{
    
}

void _DroneBoxJSON::actionReuqest(picojson::object& jo)
{
    
}

void _DroneBoxJSON::actionComplete(picojson::object& jo)
{
    
}

void _DroneBoxJSON::actionReply(picojson::object& jo)
{
    
}

void _DroneBoxJSON::draw ( void )
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
