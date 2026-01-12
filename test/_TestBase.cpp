#include "_TestBase.h"

namespace kai
{

    _TestBase::_TestBase()
    {
    }

    _TestBase::~_TestBase()
    {
    }

    int _TestBase::init(const json& j)
    {
        CHECK_(this->_ModuleBase::init(j));
        Kiss *pK = (Kiss *)pKiss;

//        = j.value("", &);

        return true;
    }

	int _TestBase::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));

		return true;
	}

    int _TestBase::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _TestBase::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _TestBase::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();



        }
    }

    void _TestBase::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        // string msg;
        // ((_Console *)pConsole)->addMsg(msg, 1);
    }

}
