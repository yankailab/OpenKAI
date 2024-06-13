/*
 * _IsingSolver.cpp
 *
 *  Created on: Feb 2, 2024
 *      Author: yankai
 */

#include "_IsingSolver.h"

namespace kai
{

    _IsingSolver::_IsingSolver()
    {
        m_pTPP = NULL;

    }

    _IsingSolver::~_IsingSolver()
    {
        DEL(m_pTPP);
    }

    bool _IsingSolver::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

//		pK->v("URI", &m_devURI);

        return true;
    }

    bool _IsingSolver::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        // n = "";
        // pK->v("_SHMrgb", &n);
        // m_psmRGB = (SharedMem *)(pK->findModule(n));

        return true;
    }

	bool _IsingSolver::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _IsingSolver::check(void)
	{
//		NULL__(, -1);
		return this->_ModuleBase::check();
	}

	void _IsingSolver::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateSolver();

			m_pT->autoFPSto();
		}
	}

	void _IsingSolver::updateSolver(void)
	{
		IF_(check() < 0);

	}

	void _IsingSolver::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
//		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
	}

}
