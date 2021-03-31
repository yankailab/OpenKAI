#ifdef USE_OPEN3D
#include "_PCscan.h"

namespace kai
{

    _PCscan::_PCscan()
    {
        m_pTui = NULL;
        m_pSB = NULL;
    }

    _PCscan::~_PCscan()
    {
        DEL(m_pTui);
    }

    bool _PCscan::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        string n = "";
        pK->v("_SlamBase", &n);
        m_pSB = (_SlamBase *)(pK->getInst(n));

        vector<string> vPCB;
        pK->a("vPCB", &vPCB);
        for (int i = 0; i < vPCB.size(); i++)
        {
            _PCbase *pP = (_PCbase *)(pK->getInst(vPCB[i]));
            IF_CONT(!pP);
            m_vPCB.push_back(pP);
        }

        Kiss *pKt = pK->child("threadUI");
        IF_F(pKt->empty());
        m_pTui = new _Thread();
        if (!m_pTui->init(pKt))
        {
            DEL(m_pTui);
            return false;
        }

        return true;
    }

    bool _PCscan::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTui);
        IF_F(!m_pT->start(getUpdate, this));
        return m_pTui->start(getUpdateUI, this);
    }

    int _PCscan::check(void)
    {
        NULL__(m_pSB, -1);
        return this->_ModuleBase::check();
    }

    void _PCscan::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();

            updateIMU();

            m_pT->autoFPSto();
        }
    }

    void _PCscan::updateIMU(void)
    {
        IF_(check() < 0);

        for (int i = 0; i < m_vPCB.size(); i++)
        {
            _PCbase *pP = m_vPCB[i];
            pP->setTranslation(m_pSB->mT());
        }
    }

    void _PCscan::updateUI(void)
    {
        while (m_pTui->bRun())
        {
            m_pTui->sleepT(0); //wait for the IObase to wake me up when received data
        }
    }

    void _PCscan::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif
