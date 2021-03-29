#ifdef USE_OPEN3D
#include "_MultiRsScan.h"

namespace kai
{

_MultiRsScan::_MultiRsScan()
{
    m_pTr = NULL;
}

_MultiRsScan::~_MultiRsScan()
{
    DEL(m_pTr);
}

bool _MultiRsScan::init ( void* pKiss )
{
    IF_F ( !this->_JSONbase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    vector<string> vPCB;
    pK->a ( "PCB", &vPCB );

    for ( int i=0; i<vPCB.size(); i++ )
    {
        _PCbase* pP = ( _PCbase* ) ( pK->getInst ( vPCB[i] ) );
        IF_CONT ( !pP );
        m_vPCB.push_back ( pP );
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

bool _MultiRsScan::start ( void )
{
    NULL_F(m_pT);
    NULL_F(m_pTr);
    IF_F(!m_pT->start(getUpdateW, this));
	return m_pTr->start(getUpdateR, this);
}

int _MultiRsScan::check ( void )
{
    return this->_JSONbase::check();
}

void _MultiRsScan::updateW ( void )
{
    while(m_pT->bRun())
    {
        if ( !m_pIO )
        {
            m_pT->sleepT ( SEC_2_USEC );
            continue;
        }

        if ( !m_pIO->isOpen() )
        {
            if ( !m_pIO->open() )
            {
                m_pT->sleepT ( SEC_2_USEC );
                continue;
            }
        }

        m_pT->autoFPSfrom();

        send();

        m_pT->autoFPSto();
    }
}

void _MultiRsScan::send ( void )
{
    IF_ ( check() <0 );

    int nC = 0;
    for ( int i=0; i<m_vPCB.size(); i++ )
    {
        _PCbase* pP = m_vPCB[i];
//        IF_CONT ( pP->size() <= 0 );
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

void _MultiRsScan::updateR ( void )
{
    while ( m_pTr->bRun() )
    {
        recv();
        m_pTr->sleepT ( 0 ); //wait for the IObase to wake me up when received data
    }
}

bool _MultiRsScan::recv()
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

void _MultiRsScan::handleMsg ( string& str )
{
    string err;
    const char* jsonstr = str.c_str();
    value json;
    parse ( json, jsonstr, jsonstr + strlen ( jsonstr ), &err );
    IF_ ( !json.is<object>() );

    object& jo = json.get<object>();
    string cmd = jo["cmd"].get<string>();
    string pct = jo["pct"].get<string>();

    _PCbase* pPCB = findPC ( pct );

    if ( cmd == "save_kiss" )
    {
        NULL_ ( pPCB );
//        pPCB->saveParam();
    }
    else if ( cmd == "var_tr" )
    {
        NULL_ ( pPCB);

        vDouble3 vT;
        vT.x = ( float ) jo["tX"].get<double>();
        vT.y = ( float ) jo["tY"].get<double>();
        vT.z = ( float ) jo["tZ"].get<double>();

        vDouble3 vR;
        vR.x = ( float ) jo["rX"].get<double>();
        vR.y = ( float ) jo["rY"].get<double>();
        vR.z = ( float ) jo["rZ"].get<double>();

        pPCB->setTranslation(vT, vR);
    }
    else if ( cmd == "save_ply" )
    {
        NULL_ ( pPCB );
    }
    else if ( cmd == "filter_" )
    {
        NULL_ ( pPCB );
    }
    else if ( cmd == "on_autoAlign" )
    {
        NULL_ ( pPCB );
    }
    else if ( cmd == "off_autoAlign" )
    {
        NULL_ ( pPCB );
    }
}

_PCbase* _MultiRsScan::findPC ( string& n )
{
    for ( int i=0; i<m_vPCB.size(); i++ )
    {
        _PCbase* pP = m_vPCB[i];
        IF_CONT ( *pP->getName() != n );

        return pP;
    }

    return NULL;
}

void _MultiRsScan::draw ( void )
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
