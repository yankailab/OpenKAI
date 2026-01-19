/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectorBase.h"

namespace kai
{

	_DetectorBase::_DetectorBase()
	{
		m_pV = nullptr;
		m_pU = nullptr;

		m_fModel = "";
		m_fWeight = "";
		m_fMean = "";
		m_fClass = "";
	}

	_DetectorBase::~_DetectorBase()
	{
	}

	bool _DetectorBase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		// model
		jKv(j, "fModel", m_fModel);
		jKv(j, "fWeight", m_fWeight);
		jKv(j, "fMean", m_fMean);
		jKv(j, "fClass", m_fClass);

		// statistics
		if (!m_fClass.empty())
		{
			ifstream ifs(m_fClass.c_str());
			IF_F(!ifs.is_open());

			string line;
			while (std::getline(ifs, line))
			{
				m_vClass.push_back(line);
			}

			ifs.close();
		}
		else
		{
			jKv(j, "vClass", m_vClass);
		}

		return true;
	}

	bool _DetectorBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n;

		n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));

		n = "";
		jKv(j, "_Universe", n);
		m_pU = (_Universe *)(pM->findModule(n));

		return true;
	}

	bool _DetectorBase::loadModel(void)
	{
		return true;
	}

	bool _DetectorBase::check(void)
	{
		return this->_ModuleBase::check();
	}

	void _DetectorBase::onPause(void)
	{
		this->_ModuleBase::onPause();
		m_pU->clear();
	}

	int _DetectorBase::getClassIdx(string &className)
	{
		for (int i = 0; i < m_vClass.size(); i++)
		{
			if (m_vClass[i] == className)
				return i;
		}

		return -1;
	}

	string _DetectorBase::getClassName(int iClass)
	{
		if (iClass < 0)
			return "";
		if (iClass >= m_vClass.size())
			return "";

		return m_vClass[iClass];
	}

	_Universe *_DetectorBase::getU(void)
	{
		return m_pU;
	}

	void _DetectorBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
	}

}
