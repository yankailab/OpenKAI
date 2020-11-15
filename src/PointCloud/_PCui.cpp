#include "_PCui.h"

#ifdef USE_OPEN3D
namespace kai
{

_PCui::_PCui()
{
}

_PCui::~_PCui()
{
}

bool _PCui::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    return true;
}

bool _PCui::start ( void )
{
    int retCode;

    if ( !m_bThreadON ) {
        m_bThreadON = true;
        retCode = pthread_create ( &m_threadID, 0, getUpdateThreadW, this );
        if ( retCode != 0 ) {
            LOG_E ( retCode );
            m_bThreadON = false;
            return false;
        }
    }

    if ( !m_bRThreadON ) {
        m_bRThreadON = true;
        retCode = pthread_create ( &m_rThreadID, 0, getUpdateThreadR, this );
        if ( retCode != 0 ) {
            LOG_E ( retCode );
            m_bRThreadON = false;
            return false;
        }
    }

    return true;
}

int _PCui::check ( void )
{
    return this->_JSONbase::check();
}

void _PCui::updateW ( void )
{
    while ( m_bThreadON ) {
        if ( !m_pIO ) {
            this->sleepTime ( USEC_1SEC );
            continue;
        }

        if ( !m_pIO->isOpen() ) {
            if ( !m_pIO->open() ) {
                this->sleepTime ( USEC_1SEC );
                continue;
            }
        }

        this->autoFPSfrom();

        send();

        this->autoFPSto();
    }
}

void _PCui::send ( void )
{
    IF_ ( check() <0 );

    picojson::object o;
    o.insert ( make_pair ( "name", value ( *this->getName() ) ) );
//	o.insert(make_pair("nP", value()));
//	o.insert(make_pair("nCam", value()));

    string msg = picojson::value ( o ).serialize() + m_msgFinishSend;
    m_pIO->write ( ( unsigned char* ) msg.c_str(), msg.size() );
    LOG_I ( "Sent: " + msg );
}

void _PCui::updateR ( void )
{
    while ( m_bRThreadON ) {
        recv();
        this->sleepTime ( 0 ); //wait for the IObase to wake me up when received data
    }
}

bool _PCui::recv()
{
    IF_F ( check() <0 );

    static string s_strB = "";

    unsigned char B;
    unsigned int nStrFinish = m_msgFinishRecv.length();
    int nB;

    while ( ( nB = m_pIO->read ( &B, 1 ) ) > 0 ) {
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

void _PCui::handleMsg ( string& str )
{
    string err;
    const char* jsonstr = str.c_str();
    value json;
    parse ( json, jsonstr, jsonstr + strlen ( jsonstr ), &err );
    IF_ ( !json.is<object>() );

    object& jo = json.get<object>();
    string cmd = jo["cmd"].get<string>();
    string pct = jo["pct"].get<string>();

    _PCtransform* pPCT = findTransform ( pct );

    if ( cmd == "save_kiss" ) {
        NULL_ ( pPCT );
        pPCT->saveParamKiss();
    } else if ( cmd == "var_tr" ) {
        NULL_ ( pPCT );

        vFloat3 v;
        v.x = ( float ) jo["tX"].get<double>();
        v.y = ( float ) jo["tY"].get<double>();
        v.z = ( float ) jo["tZ"].get<double>();
        pPCT->setTranslation ( v );
        v.x = ( float ) jo["rX"].get<double>();
        v.y = ( float ) jo["rY"].get<double>();
        v.z = ( float ) jo["rZ"].get<double>();
        pPCT->setRotation ( v );
    } else if ( cmd == "save_ply" ) {
        NULL_ ( pPCT );
    } else if ( cmd == "filter_" ) {
        NULL_ ( pPCT );
    } else if ( cmd == "on_autoAlign" ) {
        NULL_ ( pPCT );
    } else if ( cmd == "off_autoAlign" ) {
        NULL_ ( pPCT );
    }
}

_PCtransform* _PCui::findTransform ( string& n )
{
    _PCtransform* pT = ( _PCtransform* ) ( ( ( Kiss* ) m_pKiss )->getInst ( n ) );
    NULL_N ( pT );

    return pT;
}

void _PCui::draw ( void )
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
#endif
