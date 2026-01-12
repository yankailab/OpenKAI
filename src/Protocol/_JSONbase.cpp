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

    int _JSONbase::init(const json& j)
    {
        CHECK_(this->_ProtocolBase::init(j));
        Kiss *pK = (Kiss *)pKiss;

        = j.value("msgFinishSend", &m_msgFinishSend);
        = j.value("msgFinishRecv", &m_msgFinishRecv);

        int v = SEC_2_USEC;
        = j.value("ieSendHB", &v);
        m_ieSendHB.init(v);

        return true;
    }

	int _JSONbase::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ProtocolBase::link(j, pM));

		return true;
	}

    int _JSONbase::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        CHECK_(m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    int _JSONbase::check(void)
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

    bool _JSONbase::sendJson(picojson::object &o)
    {
        IF_F(!check());

        string msg = picojson::value(o).serialize() + m_msgFinishSend;
        return m_pIO->write((unsigned char *)msg.c_str(), msg.size());
    }

    void _JSONbase::sendHeartbeat(void)
    {
        object o;
        JO(o, "id", i2str(1));
        JO(o, "cmd", "heartbeat");
        JO(o, "t", li2str(m_pT->getTfrom()));

        sendJson(o);
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

    bool _JSONbase::recvJson(string* pStr, _IObase* pIO)
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
        value json;
        IF_(!str2JSON(str, &json));

        object &jo = json.get<object>();
        string cmd = jo["cmd"].get<string>();
    }

    bool _JSONbase::str2JSON(const string &str, picojson::value *pJson)
    {
        NULL_F(pJson);

        string err;
        const char *jsonstr = str.c_str();
        parse(*pJson, jsonstr, jsonstr + strlen(jsonstr), &err);
        IF_F(!pJson->is<object>());

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
