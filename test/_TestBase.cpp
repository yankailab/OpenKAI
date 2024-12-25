#include "_TestBase.h"

namespace kai
{

    _TestBase::_TestBase()
    {
    }

    _TestBase::~_TestBase()
    {
    }

    int _TestBase::init(void *pKiss)
    {
        CHECK_(this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

//        pK->v("", &);

        return OK_OK;
    }

	int _TestBase::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		return OK_OK;
	}

    int _TestBase::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
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
