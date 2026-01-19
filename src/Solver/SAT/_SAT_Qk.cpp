/*
 * _SAT_Qk.cpp
 *
 *  Created on: Feb 2, 2024
 *      Author: yankai
 */

#include "_SAT_Qk.h"

namespace kai
{

	_SAT_Qk::_SAT_Qk()
	{
		m_pTpp = nullptr;
	}

	_SAT_Qk::~_SAT_Qk()
	{
		DEL(m_pTpp);
	}

	bool _SAT_Qk::init(const json &j)
	{
		IF_F(!_ModuleBase::init(j));

		return true;
	}

	bool _SAT_Qk::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		return true;
	}

	bool _SAT_Qk::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _SAT_Qk::check(void)
	{
		return this->_ModuleBase::check();
	}

	void _SAT_Qk::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateSolver();
		}
	}

	void _SAT_Qk::updateSolver(void)
	{
		IF_(!check());

		runQ();
	}

	void _SAT_Qk::runQ(void)
	{
		uint32_t n = 100;
		QkObs *obs = qk_obs_zero(n);

		complex<double> coeff = 2;
		QkBitTerm bit_terms[3] = {QkBitTerm_X, QkBitTerm_Y, QkBitTerm_Z};
		uint32_t idx[3] = {0, 1, 2};
		//		QkObsTerm term = {coeff, 3, bit_terms, idx, n};
		QkObsTerm term = {{2, 0}, 3, bit_terms, idx, n};
		qk_obs_add_term(obs, &term);

		printf("num_qubits: %u\n", qk_obs_num_qubits(obs));
		printf("num_terms: %lu\n", qk_obs_num_terms(obs));
		printf("observable: %s\n", qk_obs_str(obs));
		qk_obs_free(obs);
	}

	void _SAT_Qk::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
	}

}
