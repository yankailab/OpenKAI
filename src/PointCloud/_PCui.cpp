#include "_PCui.h"

#ifdef USE_OPEN3D
namespace kai
{

_PCui::_PCui()
{
    m_pTr = NULL;
}

_PCui::~_PCui()
{
    DEL(m_pTr);
}

bool _PCui::init ( void* pKiss )
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

bool _PCui::start ( void )
{
    NULL_F(m_pT);
    NULL_F(m_pTr);
    IF_F(!m_pT->start(getUpdateW, this));
	return m_pTr->start(getUpdateR, this);
}

int _PCui::check ( void )
{
    return this->_JSONbase::check();
}

void _PCui::updateW ( void )
{
    while(m_pT->bRun())
    {
        if ( !m_pIO )
        {
            m_pT->sleepTime ( USEC_1SEC );
            continue;
        }

        if ( !m_pIO->isOpen() )
        {
            if ( !m_pIO->open() )
            {
                m_pT->sleepTime ( USEC_1SEC );
                continue;
            }
        }

        m_pT->autoFPSfrom();

        send();

        m_pT->autoFPSto();
    }
}

void _PCui::send ( void )
{
    IF_ ( check() <0 );

    int nC = 0;
    for ( int i=0; i<m_vPCT.size(); i++ )
    {
        _PCtransform* pP = m_vPCT[i];
        IF_CONT ( pP->size() <= 0 );
        nC++;
    }

    if ( nC <= 0 )
    {
        picojson::array ao;
        for ( unsigned int i=0; i<108; i++ )
        {
            object o;
            o.insert ( make_pair ( "id", value ( "tf" + i2str ( i ) ) ) );
            o.insert ( make_pair ( "state", value ( "OFF" ) ) );
            o.insert ( make_pair ( "cpu", value ( "N/A" ) ) );
            o.insert ( make_pair ( "mem", value ( "N/A" ) ) );
            o.insert ( make_pair ( "str", value ( "N/A" ) ) );
            o.insert ( make_pair ( "pcn", value ( "N/A" ) ) );

            ao.push_back ( value ( o ) );
        }

        string msg = picojson::value ( ao ).serialize();
        _WebSocket* pWS = ( _WebSocket* ) m_pIO;
        pWS->write ( ( unsigned char* ) msg.c_str(), msg.size(), WS_MODE_TXT );
        return;
    }

    picojson::array ao;
    for ( unsigned int i=0; i<108; i++ )
    {
        object o;
        o.insert ( make_pair ( "id", value ( "tf" + i2str ( i ) ) ) );
        o.insert ( make_pair ( "state", value ( "NORMAL" ) ) );
        o.insert ( make_pair ( "cpu", value ( "MAXN/"+f2str ( 1.4 + NormRand() * 0.25 )+"GHz" ) ) );
        o.insert ( make_pair ( "mem", value ( "Total:4GB, Available:"+ f2str ( 2 + NormRand() * 0.2 ) +"GB" ) ) );
        o.insert ( make_pair ( "str", value ( "64GB" ) ) );
        o.insert ( make_pair ( "pcn", value ( i2str ( 921600 + NormRand() * 100000.0 ) ) ) );

        ao.push_back ( value ( o ) );
    }

    string msg = picojson::value ( ao ).serialize();// + m_msgFinishSend;
    _WebSocket* pWS = ( _WebSocket* ) m_pIO;
    pWS->write ( ( unsigned char* ) msg.c_str(), msg.size(), WS_MODE_TXT );
}

void _PCui::updateR ( void )
{
    while ( m_pTr->bRun() )
    {
        recv();
        m_pTr->sleepTime ( 0 ); //wait for the IObase to wake me up when received data
    }
}

bool _PCui::recv()
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

    if ( cmd == "save_kiss" )
    {
        NULL_ ( pPCT );
        pPCT->saveParamKiss();
    }
    else if ( cmd == "var_tr" )
    {
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

_PCtransform* _PCui::findTransform ( string& n )
{
    for ( int i=0; i<m_vPCT.size(); i++ )
    {
        _PCtransform* pP = m_vPCT[i];
        IF_CONT ( *pP->getName() != n );

        return pP;
    }

    return NULL;
}

void _PCui::draw ( void )
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
