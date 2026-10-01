#include "_JSONbase.h"
#include <openssl/evp.h>

namespace kai
{

    _JSONbase::_JSONbase()
    {
    }

    _JSONbase::~_JSONbase()
    {
    }

    bool _JSONbase::loadConfig(void)
    {
        IF_F(!this->_ProtocolBase::loadConfig());
        const json &j = *m_pJ;

        jKv(j, "msgFinishSend", m_msgFinishSend);
        jKv(j, "msgFinishRecv", m_msgFinishRecv);

        uint64_t v = NSEC_SEC;
        jKv(j, "ieSendHB", v);
        m_ieSendHB.init(v);

        return true;
    }

    bool _JSONbase::saveConfig(bool bExport)
    {
        IF_F(!_ProtocolBase::saveConfig(false));

        json &j = *m_pJ;
        j["msgFinishSend"] = m_msgFinishSend;
        j["msgFinishRecv"] = m_msgFinishRecv;
        j["ieSendHB"] = m_ieSendHB.m_tInterval;

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _JSONbase::link(InstanceMgr *pM)
    {
        IF_F(!this->_ProtocolBase::link(pM));

        return true;
    }

    bool _JSONbase::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->startThread(getUpdateW, this));
        return m_pTr->startThread(getUpdateR, this);
    }

    bool _JSONbase::check(void)
    {
        return this->_ProtocolBase::check();
    }

    void _JSONbase::updateW(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            send();
        }
    }

    void _JSONbase::send(void)
    {
        IF_(!check());

        if (m_ieSendHB.update(m_pT->getTfromNs()))
        {
            // sendHeartbeat();
        }
    }

    bool _JSONbase::sendJson(const json &j)
    {
        IF_F(!check());
        IF_F(!j.is_object());

        string msg = j.dump() + m_msgFinishSend;
        NULL_F(m_pBpStreamOut);
        m_pBpStreamOut->addPacket(vector<uint8_t>(msg.begin(), msg.end()));
        return true;
    }

    void _JSONbase::sendHeartbeat(void)
    {
        json j = json::object();
        j["id"] = i2str(1);
        j["cmd"] = "heartbeat";
        j["t"] = li2str(m_pT->getTfromNs());

        sendJson(j);
    }

    void _JSONbase::updateR(void)
    {
        string strR = "";

        while (m_pTr->bRun())
        {
            if (!recvJson(&strR))
            {
                m_pTr->autoFPS();
                continue;
            }

            handleJson(strR);
            strR.clear();
            m_nCMDrecv++;
        }
    }

    bool _JSONbase::recvJson(string *pStr)
    {
        IF_F(!check());
        NULL_F(pStr);

        const size_t nStrFinish = m_msgFinishRecv.length();
        uint8_t b;
        while (readByte(&b))
        {
            *pStr += b;
            IF_CONT(pStr->length() <= nStrFinish);
            IF_CONT(pStr->compare(pStr->length() - nStrFinish, nStrFinish, m_msgFinishRecv) != 0);

            pStr->erase(pStr->length() - nStrFinish, nStrFinish);
            LOG_I("Received: " + *pStr);
            return true;
        }

        return false;
    }

    void _JSONbase::handleJson(const string &str)
    {
    }

    bool _JSONbase::str2JSON(const string &str, json &j)
    {
        JsonCfg jCfg;
        IF_F(!jCfg.parseStr(str));
        j = *jCfg.getJson();

        IF_F(!j.is_object());

        return true;
    }

    void _JSONbase::md5(const string &str, string *pDigest)
    {
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int nDigest = 0;
        int r = EVP_Digest(str.data(), str.length(), digest, &nDigest, EVP_md5(), nullptr);
        IF_(!r);

        string strD(reinterpret_cast<char *>(digest), nDigest);
        *pDigest = strD;
        LOG_I("md5: " + *pDigest);
    }

    void _JSONbase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ProtocolBase::console(pConsole);

        ((_Console *)pConsole)->addMsg(check() ? "BytePacketStream linked" : "BytePacketStream unavailable", 1);
    }

}
