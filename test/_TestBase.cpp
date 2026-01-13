#include "_TestBase.h"

namespace kai
{

    _TestBase::_TestBase()
    {
    }

    _TestBase::~_TestBase()
    {
    }

    bool _TestBase::init(const json& j)
    {
        IF_F(!this->_ModuleBase::init(j));

//        = j.value("", &);

        return true;
    }

	bool _TestBase::link(const json& j, ModuleMgr* pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		return true;
	}

    bool _TestBase::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    bool _TestBase::check(void)
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
