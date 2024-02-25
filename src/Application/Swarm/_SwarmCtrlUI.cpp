#include "_SwarmCtrlUI.h"

namespace kai
{

    _SwarmCtrlUI::_SwarmCtrlUI()
    {
        m_Tr = NULL;
        m_pCtrl = NULL;
        m_pSwarm = NULL;
    }

    _SwarmCtrlUI::~_SwarmCtrlUI()
    {
        DEL(m_pTr);
    }

    bool _SwarmCtrlUI::init(void *pKiss)
    {
        IF_F(!this->_JSONbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        int v;
        v = SEC_2_USEC;
        pK->v("ieSendHB", &v);
        m_ieSendHB.init(v);

        v = SEC_2_USEC;
        pK->v("ieSendNodeUpdate", &v);
        m_ieSendNodeUpdate.init(v);

        v = SEC_2_USEC;
        pK->v("ieSendNodeClearAll", &v);
        m_ieSendNodeClearAll.init(v);

        Kiss *pKt = pK->child("threadR");
        IF_F(pKt->empty());

        m_pTr = new _Thread();
        if (!m_pTr->init(pKt))
        {
            DEL(m_pTr);
            return false;
        }

        return true;
    }

    bool _SwarmCtrlUI::link(void)
    {
        IF_F(!this->_JSONbase::link());
        IF_F(!m_pTr->link());

        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_SwarmCtrl", &n);
        m_pCtrl = (_SwarmCtrl *)(pK->getInst(n));
        IF_Fl(!m_pCtrl, n + ": not found");

        n = "";
        pK->v("_SwarmSearch", &n);
        m_pSwarm = (_SwarmSearch *)(pK->getInst(n));
        IF_Fl(!m_pSwarm, n + ": not found");

        return true;
    }

    bool _SwarmCtrlUI::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTr);
        IF_F(!m_pT->start(getUpdateW, this));
        return m_pTr->start(getUpdateR, this);
    }

    int _SwarmCtrlUI::check(void)
    {
        NULL__(m_pCtrl, -1);
        NULL__(m_pSwarm, -1);

        return this->_JSONbase::check();
    }

    void _SwarmCtrlUI::updateW(void)
    {
        while (m_pT->bThread())
        {
            if (!m_pIO)
            {
                m_pT->sleepT(SEC_2_USEC);
                continue;
            }

            m_pT->autoFPSfrom();

            send();

            m_pT->autoFPSto();
        }
    }

    void _SwarmCtrlUI::send(void)
    {
        IF_(check() < 0);

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
        IF_(check() < 0);

        object o;
        JO(o, "cmd", "hb");
        JO(o, "id", (double)m_pCtrl->m_node.m_id);
        JO(o, "s", m_pCtrl->m_state.getState());

        sendMsg(o);
    }

    void _SwarmCtrlUI::sendNodeUpdate(void)
    {
        IF_(check() < 0);

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
        IF_(check() < 0);

        object o;
        JO(o, "cmd", "ndClearAll");
        JO(o, "id", (double)m_pCtrl->m_node.m_id);

        sendMsg(o);
    }

    void _SwarmCtrlUI::updateR(void)
    {
        while (m_pTr->bRun())
        {
            m_pTr->autoFPSfrom();

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
        IF_(check() < 0);

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
