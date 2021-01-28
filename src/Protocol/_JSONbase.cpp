#include "_JSONbase.h"

namespace kai
{

_JSONbase::_JSONbase()
{
    m_rThreadID = 0;
    m_bRThreadON = false;
    m_pIO = NULL;

    m_msgFinishSend = "SWOOLEFN";
    m_msgFinishRecv = "SWOOLE_SOCKET_FINISH";
}

_JSONbase::~_JSONbase()
{
}

bool _JSONbase::init ( void* pKiss )
{
    IF_F ( !this->_ThreadBase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "msgFinishSend", &m_msgFinishSend );
    pK->v ( "msgFinishRecv", &m_msgFinishRecv );

    string n;
    n = "";
    F_ERROR_F ( pK->v ( "_IOBase", &n ) );
    m_pIO = ( _IOBase* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pIO,"_IOBase not found" );

    return true;
}

bool _JSONbase::start ( void )
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

int _JSONbase::check ( void )
{
    NULL__ ( m_pIO, -1 );
    IF__ ( !m_pIO->isOpen(), -1 );

    return 0;
}

void _JSONbase::updateW ( void )
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

void _JSONbase::send ( void )
{
    IF_ ( check() <0 );

    if(m_tIntHeartbeat.update(m_tStamp))
    {
        sendHeartbeat();
    }
}

bool _JSONbase::sendMsg (picojson::object& o)
{
    string msg = picojson::value ( o ).serialize() + m_msgFinishSend;
    return m_pIO->write ( ( unsigned char* ) msg.c_str(), msg.size() );  
}

bool _JSONbase::sendHeartbeat (void)
{
    object o;
    JO(o, "id", i2str(1));
    JO(o, "t", li2str(m_tStamp));
    
    return sendMsg(o);
}

void _JSONbase::updateR ( void )
{
    while ( m_bRThreadON )
    {
        if ( recv() )
        {
            handleMsg ( m_strB );
            m_strB.clear();
        }
        this->sleepTime ( 0 ); //wait for the IObase to wake me up when received data
    }
}

bool _JSONbase::recv()
{
    IF_F ( check() <0 );

    unsigned char B;
    unsigned int nStrFinish = m_msgFinishRecv.length();

    while ( m_pIO->read ( &B, 1 ) > 0 )
    {
        m_strB += B;
        IF_CONT ( m_strB.length() <= nStrFinish );

        string lstr = m_strB.substr ( m_strB.length()-nStrFinish, nStrFinish );
        IF_CONT ( lstr != m_msgFinishRecv );

        m_strB.erase ( m_strB.length()-nStrFinish, nStrFinish );
        LOG_I ( "Received: " + m_strB );
        return true;
    }

    return false;
}

void _JSONbase::handleMsg ( string& str )
{
    value json;    
    IF_(!str2JSON(str,&json));
    
    object& jo = json.get<object>();
    string cmd = jo["cmd"].get<string>();
}

bool _JSONbase::str2JSON(string& str, picojson::value* pJson)
{
    NULL_F(pJson);
    
    string err;
    const char* jsonstr = str.c_str();
    parse ( *pJson, jsonstr, jsonstr + strlen ( jsonstr ), &err );
    IF_F ( !pJson->is<object>() );
    
    return true;
}

void _JSONbase:: md5( string& str, string* pDigest )
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5 ( ( const unsigned char* ) str.c_str(), str.length(), digest );
    
    string strD((char*)digest);
    *pDigest = strD;
    LOG_I( "md5: " + *pDigest );
}

void _JSONbase::draw ( void )
{
    this->_ThreadBase::draw();

    string msg = *this->getName();
    if ( m_pIO->isOpen() )
        msg += ": Connected";
    else
        msg += ": Not connected";

    addMsg ( msg );

}

}
