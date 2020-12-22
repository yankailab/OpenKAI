#include "_UIhandlerBase.h"

namespace kai
{

_UIhandlerBase::_UIhandlerBase()
{
}

_UIhandlerBase::~_UIhandlerBase()
{
}

bool _UIhandlerBase::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    return true;
}

bool _UIhandlerBase::start ( void )
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

int _UIhandlerBase::check ( void )
{
    return this->_JSONbase::check();
}

void _UIhandlerBase::updateW ( void )
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

void _UIhandlerBase::send ( void )
{
    IF_ ( check() <0 );

//  picojson::object o;
//  o.insert ( make_pair ( "id", value ( *this->getName() ) ) );
//	o.insert(make_pair("nP", value()));
//	o.insert(make_pair("nCam", value()));
    
//  picojson::object o;
//	o.insert(make_pair("action", value(string("start_fly"))));

	picojson::array ao;
	for(unsigned int i=0; i<100; i++)
	{
		object o;
		o.insert(make_pair("id", value(i2str(1))));
		o.insert(make_pair("state", value("NORMAL")));
		o.insert(make_pair("cpu", value("MAXN/1.4GHz")));
		o.insert(make_pair("mem", value("4GB")));
		o.insert(make_pair("str", value("64GB")));
		o.insert(make_pair("pcn", value("921600")));

		ao.push_back(value(o));
	}
//	o.insert(make_pair("waypoints", value(wpA)));

    string msg = picojson::value ( ao ).serialize();// + m_msgFinishSend;

    _WebSocket* pWS = (_WebSocket*)m_pIO;
    pWS->write ( ( unsigned char* ) msg.c_str(), msg.size(), WS_MODE_TXT );
    LOG_I ( "Sent: " + msg );
}

void _UIhandlerBase::updateR ( void )
{
    while ( m_bRThreadON )
    {
        recv();
        this->sleepTime ( 0 ); //wait for the IObase to wake me up when received data
    }
}

bool _UIhandlerBase::recv()
{
    IF_F ( check() <0 );

    static string s_strB = "";

    unsigned char B;
    unsigned int nStrFinish = m_msgFinishRecv.length();
    int nB;

    while ( ( nB = m_pIO->read ( &B, 1 ) ) > 0 )
    {
        s_strB += B;
        IF_CONT ( s_strB.length() <= nStrFinish );

        string lstr = s_strB.substr ( s_strB.length()-nStrFinish, nStrFinish );
        IF_CONT ( lstr != m_msgFinishRecv );

        s_strB.erase ( s_strB.length()-nStrFinish, nStrFinish );
        handleMsg ( s_strB );

        LOG_I ( "Received: " + s_strB );
        s_strB.clear();
    }

    return false;
}

void _UIhandlerBase::handleMsg ( string& str )
{
    string err;
    const char* jsonstr = str.c_str();
    value json;
    parse ( json, jsonstr, jsonstr + strlen ( jsonstr ), &err );
    IF_ ( !json.is<object>() );

    object& jo = json.get<object>();
    string cmd = jo["cmd"].get<string>();
    string pct = jo["pct"].get<string>();

    if ( cmd == "save_kiss" )
    {
    }
    else if ( cmd == "var_tr" )
    {
    }
}

void _UIhandlerBase::draw ( void )
{
    this->_ThreadBase::draw();

    string msg = *this->getName();
    if ( m_pIO->isOpen() )
        msg += ": CONNECTED";
    else
        msg += ": Not connected";

    addMsg ( msg );

}

}
