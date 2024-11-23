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

    int _JSONbase::init(void *pKiss)
    {
        CHECK_(this->_ProtocolBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("msgFinishSend", &m_msgFinishSend);
        pK->v("msgFinishRecv", &m_msgFinishRecv);

        int v = SEC_2_USEC;
        pK->v("ieSendHB", &v);
        m_ieSendHB.init(v);

        return OK_OK;
    }

	int _JSONbase::link(void)
	{
		CHECK_(this->_ProtocolBase::link());

		return OK_OK;
	}

    int _JSONbase::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        NULL__(m_pTr, OK_ERR_NULLPTR);
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
            m_pT->autoFPSfrom();

            send();

            m_pT->autoFPSto();
        }
    }

    void _JSONbase::send(void)
    {
        IF_(check() != OK_OK);

        if (m_ieSendHB.update(m_pT->getTfrom()))
        {
//            sendHeartbeat();
        }
    }

    bool _JSONbase::sendMsg(picojson::object &o)
    {
        string msg = picojson::value(o).serialize() + m_msgFinishSend;

        return m_pIO->write((unsigned char *)msg.c_str(), msg.size());
    }

    void _JSONbase::sendHeartbeat(void)
    {
        object o;
        JO(o, "id", i2str(1));
        JO(o, "cmd", "heartbeat");
        JO(o, "t", li2str(m_pT->getTfrom()));

        sendMsg(o);
    }

    void _JSONbase::updateR(void)
    {
        string strR = "";

        while (m_pTr->bAlive())
        {
            IF_CONT(!recvJson(&strR));

            handleJson(strR);
            strR.clear();
			m_nCMDrecv++;
        }
    }

    bool _JSONbase::recvJson(string* pStr)
    {
        IF_F(check() != OK_OK);
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
