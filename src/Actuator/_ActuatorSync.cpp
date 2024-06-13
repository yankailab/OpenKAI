/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorSync.h"

namespace kai
{

	_ActuatorSync::_ActuatorSync()
	{
		m_nAB = 0;
		m_iABget = 0;
	}

	_ActuatorSync::~_ActuatorSync()
	{
	}

	bool _ActuatorSync::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		vector<string> vName;
		pK->a("_ActuatorList", &vName);

		for (int i = 0; i < vName.size(); i++)
		{
			void *p = pK->findModule(vName[i]);
			IF_CONT(!p);

			m_pAB[m_nAB] = (_ActuatorSync *)p;
			m_nAB++;
		}

		pK->v("iABget", &m_iABget);
		if (m_iABget >= m_nAB)
			m_iABget = 0;

		return true;
	}

	bool _ActuatorSync::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ActuatorSync::check(void)
	{
		return this->_ActuatorBase::check();
	}

	void _ActuatorSync::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateSync();

			m_pT->autoFPSto();
		}
	}

	void _ActuatorSync::updateSync(void)
	{
		_ActuatorBase *pA = m_pAB[m_iABget];
		NULL_(pA);
	}

	void _ActuatorSync::setPtarget(int i, float nP)
	{
		for (int i = 0; i < m_nAB; i++)
			m_pAB[i]->setPtarget(i, nP);
	}

	void _ActuatorSync::setStarget(int i, float nS)
	{
		for (int i = 0; i < m_nAB; i++)
			m_pAB[i]->setStarget(i, nS);
	}

	void _ActuatorSync::gotoOrigin(void)
	{
		for (int i = 0; i < m_nAB; i++)
			m_pAB[i]->gotoOrigin();
	}

	bool _ActuatorSync::bComplete(void)
	{
		for (int i = 0; i < m_nAB; i++)
		{
			IF_F(!m_pAB[i]->bComplete());
		}

		return true;
	}

	float _ActuatorSync::getP(int i)
	{
		return m_pAB[m_iABget]->getP(i);
	}

	float _ActuatorSync::getS(int i)
	{
		return m_pAB[m_iABget]->getS(i);
	}

}
