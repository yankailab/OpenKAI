#include "_JSONbase.h"

namespace kai
{

    _JSONbase::_JSONbase()
    {
        m_msgFinishSend = "";
        m_msgFinishRecv = "EOJ";
    }

    _JSONbase::~_JSONbase()
    {
    }

    bool _JSONbase::init(const json &j)
    {
        IF_F(!this->_ProtocolBase::init(j));

        jKv(j, "msgFinishSend", m_msgFinishSend);
        jKv(j, "msgFinishRecv", m_msgFinishRecv);

        int v = SEC_2_USEC;
        jKv(j, "ieSendHB", v);
        m_ieSendHB.init(v);

        return true;
    }

    bool _JSONbase::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ProtocolBase::link(j, pM));

        return true;
    }

    bool _JSONbase::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    bool _JSONbase::check(void)
    {
        return this->_ProtocolBase::check();
    }

    void _JSONbase::updateW(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            send();
        }
    }

    void _JSONbase::send(void)
    {
        IF_(!check());

        if (m_ieSendHB.updateT(m_pT->getTfrom()))
        {
            //            sendHeartbeat();
        }
    }

    bool _JSONbase::sendJson(const json &j)
    {
        IF_F(!check());
        IF_F(!j.is_object());

        string msg = j.dump() + m_msgFinishSend;
        return m_pIO->write((unsigned char *)msg.c_str(), msg.size());
    }

    void _JSONbase::sendHeartbeat(void)
    {
        json j = json::object();
        j["id"] = i2str(1);
        j["cmd"] = "heartbeat";
        j["t"] = li2str(m_pT->getTfrom());

        sendJson(j);
    }

    void _JSONbase::updateR(void)
    {
        string strR = "";

        while (m_pTr->bAlive())
        {
            IF_CONT(!recvJson(&strR, m_pIO));

            handleJson(strR);
            strR.clear();
            m_nCMDrecv++;
        }
    }

    bool _JSONbase::recvJson(string *pStr, _IObase *pIO)
    {
        IF_F(!check());
        NULL_F(pStr);

        if (m_nRead == 0)
        {
            m_nRead = m_pIO->read(m_pBuf, JB_N_BUF);
            IF_F(m_nRead <= 0);
            m_iRead = 0;
        }

        unsigned int nStrFinish = m_msgFinishRecv.length();

        while (m_iRead < m_nRead)
        {
            *pStr += m_pBuf[m_iRead++];
            if (m_iRead == m_nRead)
            {
                m_iRead = 0;
                m_nRead = 0;
            }

            IF_CONT(pStr->length() <= nStrFinish);

            string lstr = pStr->substr(pStr->length() - nStrFinish, nStrFinish);
            IF_CONT(lstr != m_msgFinishRecv);

            pStr->erase(pStr->length() - nStrFinish, nStrFinish);
            LOG_I("Received: " + *pStr);
            return true;
        }

        return false;
    }

    void _JSONbase::handleJson(const string &str)
    {
        // value json;
        // IF_(!str2JSON(str, &json));

        // object &jo = json.get<object>();
        // string cmd = jo["cmd"].get<string>();
    }

    bool _JSONbase::str2JSON(const string &str, json &j)
    {
        string err;
        const char *jsonstr = str.c_str();

        JsonCfg jCfg;
        jCfg.parseJsonStr(str);
        j = jCfg.getJson();

        IF_F(!j.is_object());

        return true;
    }

    void _JSONbase::md5(const string &str, string *pDigest)
    {
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5((const unsigned char *)str.c_str(), str.length(), digest);

        string strD((char *)digest);
        *pDigest = strD;
        LOG_I("md5: " + *pDigest);
    }

    void _JSONbase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ProtocolBase::console(pConsole);

        string msg;
        if (m_pIO->bOpen())
            msg = "Connected";
        else
            msg = "Not connected";

        ((_Console *)pConsole)->addMsg(msg, 1);
    }

}
