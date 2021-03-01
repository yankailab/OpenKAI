#include "_PCscan.h"

#ifdef USE_OPEN3D
namespace kai
{

_PCscan::_PCscan()
{
    m_pTr = NULL;
}

_PCscan::~_PCscan()
{
    DEL(m_pTr);
}

bool _PCscan::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    vector<string> vPCT;
    pK->a ( "PCT", &vPCT );

    for ( int i=0; i<vPCT.size(); i++ )
    {
        _PCtransform* pP = ( _PCtransform* ) ( pK->getInst ( vPCT[i] ) );
        IF_CONT ( !pP );
        m_vPCT.push_back ( pP );
    }
    
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

bool _PCscan::start ( void )
{
    NULL_F(m_pT);
    NULL_F(m_pTr);
    IF_F(!m_pT->start(getUpdateW, this));
	return m_pTr->start(getUpdateR, this);
}

int _PCscan::check ( void )
{
    return this->_JSONbase::check();
}

void _PCscan::updateW ( void )
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

void _PCscan::send ( void )
{
    IF_ ( check() <0 );

    int nC = 0;
    for ( int i=0; i<m_vPCT.size(); i++ )
    {
        _PCtransform* pP = m_vPCT[i];
        IF_CONT ( pP->size() <= 0 );
        nC++;
    }

    picojson::array ao;
    for ( unsigned int i=0; i<108; i++ )
    {
        object o;
        o.insert ( make_pair ( "id", value ( "tf" + i2str ( i ) ) ) );

        ao.push_back ( value ( o ) );
    }

    string msg = picojson::value ( ao ).serialize();// + m_msgFinishSend;
    _WebSocket* pWS = ( _WebSocket* ) m_pIO;
    pWS->write ( ( unsigned char* ) msg.c_str(), msg.size(), WS_MODE_TXT );
}

void _PCscan::updateR ( void )
{
    while ( m_pTr->bRun() )
    {
        recv();
        m_pTr->sleepT ( 0 ); //wait for the IObase to wake me up when received data
    }
}

bool _PCscan::recv()
{
    IF_F ( check() <0 );

    static string s_strB = "";
    unsigned char B;
    unsigned int nStrFinish = m_msgFinishRecv.length();

    while ( m_pIO->read ( &B, 1 ) > 0 )
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

void _PCscan::handleMsg ( string& str )
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

    if ( cmd == "save_kiss" )
    {
        NULL_ ( pPCT );
        pPCT->saveParamKiss();
    }
    else if ( cmd == "var_tr" )
    {
        NULL_ ( pPCT );

        vDouble3 v;
        v.x = ( float ) jo["tX"].get<double>();
        v.y = ( float ) jo["tY"].get<double>();
        v.z = ( float ) jo["tZ"].get<double>();
        pPCT->setTranslation ( v );
        v.x = ( float ) jo["rX"].get<double>();
        v.y = ( float ) jo["rY"].get<double>();
        v.z = ( float ) jo["rZ"].get<double>();
        pPCT->setRotation ( v );
    }
    else if ( cmd == "save_ply" )
    {
        NULL_ ( pPCT );
    }
    else if ( cmd == "filter_" )
    {
        NULL_ ( pPCT );
    }
    else if ( cmd == "on_autoAlign" )
    {
        NULL_ ( pPCT );
    }
    else if ( cmd == "off_autoAlign" )
    {
        NULL_ ( pPCT );
    }
}

_PCtransform* _PCscan::findTransform ( string& n )
{
    for ( int i=0; i<m_vPCT.size(); i++ )
    {
        _PCtransform* pP = m_vPCT[i];
        IF_CONT ( *pP->getName() != n );

        return pP;
    }

    return NULL;
}

void _PCscan::draw ( void )
{
    this->_ModuleBase::draw();

    string msg;
    if ( m_pIO->isOpen() )
        msg = "STANDBY, CONNECTED";
    else
        msg = "STANDBY, Not connected";

    addMsg ( msg );

}

}
#endif
