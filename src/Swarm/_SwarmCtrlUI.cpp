#include "_SwarmCtrlUI.h"

namespace kai
{

    _SwarmCtrlUI::_SwarmCtrlUI()
    {
        m_Tr = nullptr;
        m_pCtrl = nullptr;
        m_pSwarm = nullptr;
    }

    _SwarmCtrlUI::~_SwarmCtrlUI()
    {
        DEL(m_pTr);
    }

    bool _SwarmCtrlUI::init(const json& j)
    {
        IF_F(!this->_JSONbase::init(j));
        Kiss *pK = (Kiss *)pKiss;

        int v;
        v = SEC_2_USEC;
        v = j.value("ieSendHB", "");
        m_ieSendHB.init(v);

        v = SEC_2_USEC;
        v = j.value("ieSendNodeUpdate", "");
        m_ieSendNodeUpdate.init(v);

        v = SEC_2_USEC;
        v = j.value("ieSendNodeClearAll", "");
        m_ieSendNodeClearAll.init(v);

        Kiss *pKt = pK->child("threadR");
        if (pKt->empty())
        {
            LOG_E("threadR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTr = new _Thread();
        CHECK_d_l_(m_pTr->init(pKt), DEL(m_pTr), "threadR init failed");

        return true;
    }

    bool _SwarmCtrlUI::link(const json& j, ModuleMgr* pM)
    {
        IF_F(!this->_JSONbase::link(j, pM));
        IF_F(!m_pTr->link());

        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        n = j.value("_SwarmCtrl", "");
        m_pCtrl = (_SwarmCtrl *)(pM->findModule(n));
        NULL__(m_pCtrl);

        n = "";
        n = j.value("_SwarmSearch", "");
        m_pSwarm = (_SwarmSearch *)(pM->findModule(n));
        NULL__(m_pSwarm);

        return true;
    }

    bool _SwarmCtrlUI::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    bool _SwarmCtrlUI::check(void)
    {
        NULL__(m_pCtrl);
        NULL__(m_pSwarm);

        return this->_JSONbase::check();
    }

    void _SwarmCtrlUI::updateW(void)
    {
        while (m_pT->bAlive())
        {
            if (!m_pIO)
            {
                m_pT->sleepT(SEC_2_USEC);
                continue;
            }

            m_pT->autoFPS();

            send();


        }
    }

    void _SwarmCtrlUI::send(void)
    {
        IF_(!check());

        uint64_t t = getApproxTbootUs();

        if (m_ieSendHB.update(t))
            sendHeartbeat();

        if (m_ieSendNodeUpdate.update(t))
            sendNodeUpdate();

        // if (m_ieSendNodeClearAll.update(t))
        //     sendNodeClearAll();
    }

    void _SwarmCtrlUI::sendHeartbeat(void)
    {
        IF_(!check());

        object o;
        JO(o, "cmd", "hb");
        JO(o, "id", (double)m_pCtrl->m_node.m_id);
        JO(o, "s", m_pCtrl->m_state.getState());

        sendMsg(o);
    }

    void _SwarmCtrlUI::sendNodeUpdate(void)
    {
        IF_(!check());

        SWARM_NODE* pN = NULL;
        int i = 0;
        while((pN = m_pSwarm->getNode(i++)))
        {
            object o;
            JO(o, "cmd", "ndUpdate");
            JO(o, "id", (double)pN->m_id);
            JO(o, "lat", lf2str(pN->m_vPos.x,10));
            JO(o, "lng", lf2str(pN->m_vPos.y,10));
            JO(o, "alt", f2str((double)pN->m_alt,2));
            JO(o, "hdg", f2str((double)pN->m_hdg,2));
            JO(o, "spd", f2str((double)pN->m_spd,2));
            JO(o, "mode", (double)pN->m_mode);
            JO(o, "batt", f2str((double)pN->m_batt,2));

            sendMsg(o);
        }
    }

    void _SwarmCtrlUI::sendNodeClearAll(void)
    {
        IF_(!check());

        object o;
        JO(o, "cmd", "ndClearAll");
        JO(o, "id", (double)m_pCtrl->m_node.m_id);

        sendMsg(o);
    }

    void _SwarmCtrlUI::updateR(void)
    {
        while (m_pTr->bAlive())
        {
            m_pTr->autoFPS();

            if (recv())
            {
                handleMsg(m_strB);
                m_strB.clear();
            }

            m_pTr->autoFPSto();
        }
    }

    void _SwarmCtrlUI::handleMsg(string &str)
    {
        value json;
        IF_(!str2JSON(str, &json));

        object &jo = json.get<object>();
        IF_(!jo["cmd"].is<string>());
        string cmd = jo["cmd"].get<string>();

        if (cmd == "setState")
            setState(jo);
    }

    void _SwarmCtrlUI::setState(picojson::object &o)
    {
        IF_(!check());

        IF_(!o["id"].is<double>());
        IF_(!o["s"].is<string>());

        int vID = o["id"].get<double>();
        string state = o["s"].get<string>();

        m_pCtrl->setState(state);
    }

    void _SwarmCtrlUI::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_JSONbase::console(pConsole);

        NULL_(m_pTr);
        m_pTr->console(pConsole);
    }

}
