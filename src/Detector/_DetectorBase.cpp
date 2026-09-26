/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_DetectorBase.h"

namespace kai
{

	_DetectorBase::_DetectorBase()
	{
	}

	_DetectorBase::~_DetectorBase()
	{
	}

	bool _DetectorBase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

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

	bool _DetectorBase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["fModel"] = m_fModel;
		j["fWeight"] = m_fWeight;
		j["fMean"] = m_fMean;
		j["fClass"] = m_fClass;
		j["vClass"] = m_vClass;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _DetectorBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		string n;

		n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));

		n = "";
		jKv(j, "_Canvas", n);
		m_pCanvas = (_Canvas *)(m_pM->findModule(n));

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
		m_pCanvas->clear();
	}

	int _DetectorBase::getClassIdx(string &className)
	{
		for (size_t i = 0; i < m_vClass.size(); i++)
		{
			if (m_vClass[i] == className)
				return static_cast<int>(i);
		}

		return -1;
	}

	string _DetectorBase::getClassName(int iClass)
	{
		if (iClass < 0)
			return "";
		if (static_cast<size_t>(iClass) >= m_vClass.size())
			return "";

		return m_vClass[iClass];
	}

	_Canvas *_DetectorBase::getCanvas(void)
	{
		return m_pCanvas;
	}

	void _DetectorBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		// _Console *pC = (_Console *)pConsole;
	}

}
