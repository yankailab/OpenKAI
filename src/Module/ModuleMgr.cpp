#include "ModuleMgr.h"
#include "Module.h"

namespace kai
{

	ModuleMgr::ModuleMgr(void)
	{
	}

	ModuleMgr::~ModuleMgr(void)
	{
		cleanAll();
	}

	bool ModuleMgr::loadJsonFiles(const string &fName)
	{
		IF_Le_F(!m_vModules.empty(), "Cannot reload JSON files while modules exist");

		JsonCfg jCfg;
		IF_F(!jCfg.readFromFile(fName));

		m_vJcfg.clear();
		m_vJcfg.push_back(jCfg);

		const json *pJapp = jK(*jCfg.getJson(), "APP");
		IF__(!pJapp || !pJapp->is_object(), true);

		vector<string> vJsonFiles;
		jKv(*pJapp, "vInclude", vJsonFiles);
		for (string f : vJsonFiles)
		{
			JsonCfg jCi;
			IF_CONT(!jCi.readFromFile(f));

			m_vJcfg.push_back(jCi);
		}

		return true;
	}

	JsonCfg *ModuleMgr::findJsonCfg(const string &name)
	{
		for (size_t i = 0; i < m_vJcfg.size(); i++)
		{
			JsonCfg *pJc = &m_vJcfg[i];
			const json *pJ = jK(*pJc->getJson(), name);

			if (pJ && pJ->is_object())
			{
				return pJc;
			}
		}

		return nullptr;
	}

	bool ModuleMgr::createAll(void)
	{
		Module md;

		for (size_t i = 0; i < m_vJcfg.size(); i++)
		{
			JsonCfg *pJc = &m_vJcfg[i];
			json* pJ = pJc->getJson();
			for (auto it = pJ->begin(); it != pJ->end(); it++)
			{
				json* pJi = &it.value();
				IF_CONT(!pJi->is_object());

				string n = it.key();
				if (n.empty())
				{
					LOG_I("Module name is empty");
					continue;
				}

				if (findModule(n))
				{
					LOG_I("Module name already existed: " + n);
					continue;
				}

				string c = "";
				jKv(*pJi, "class", c);
				IF_CONT(c == "ModuleMgr");
				if (c.empty())
				{
					LOG_I("Class name is empty: " + n);
					continue;
				}

				bool bON = true;
				jKv(*pJi, "bON", bON);
				if (!bON)
				{
					LOG_I("Module disabled: " + n);
					continue;
				}

				_ModuleBase *pM = md.createInstance(c);
				if (pM == nullptr)
				{
					LOG_I("Instance not created: " + n);
					continue;
				}

				pM->setName(n);
				pM->setConfig(pJc, pJi);
				m_vModules.push_back(pM);
				LOG_I("Instance created: " + n);
			}
		}

		return true;
	}

	bool ModuleMgr::initAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			if (!pM->loadConfig())
			{
				LOG_E(pM->getName() + ".loadConfig() failed");
				return false;
			}

			LOG_I("Initialized: " + pM->getName());
		}

		return true;
	}

	bool ModuleMgr::linkAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			if (!pM->link(this))
			{
				LOG_E(pM->getName() + ".link() failed");
				return false;
			}

			LOG_I("Linked: " + pM->getName());
		}

		return true;
	}

	bool ModuleMgr::startAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			if (!pM->start())
			{
				LOG_E(pM->getName() + ".start() failed");
				return false;
			}

			LOG_I("Started: " + pM->getName());
		}

		return true;
	}

	void ModuleMgr::pauseAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			pM->pause();
		}
	}

	void ModuleMgr::resumeAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			pM->resume();
		}
	}

	void ModuleMgr::stopAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			pM->stop();
		}

		// TODO
	}

	void ModuleMgr::waitForComplete(void)
	{
		// TODO: temporal impl
		while (!bComplete())
		{
			sleep(1);
		}
	}

	bool ModuleMgr::bComplete(void)
	{
		// TODO: temporal impl
		return false;
	}

	void ModuleMgr::cleanAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			DEL(pM);
		}

		m_vModules.clear();
	}

	void *ModuleMgr::findModule(const string &name)
	{
		IF_N(name.empty());

		for (_ModuleBase *pM : m_vModules)
		{
			if (name == pM->getName())
			{
				return pM;
			}
		}

		return nullptr;
	}

	json* ModuleMgr::findJson(const string &name)
	{
		for (size_t i = 0; i < m_vJcfg.size(); i++)
		{
			JsonCfg *pJc = &m_vJcfg[i];
			json *pJ = jK(*pJc->getJson(), name);

			if (pJ && pJ->is_object())
			{
				return pJ;
			}
		}

		return nullptr;
	}

	bool ModuleMgr::addModule(void *pModule, const string &name)
	{
		NULL_F(pModule);
		IF_Le_F(findModule(name), "Module already existed: " + name);
		IF_Le_F(!findJson(name), "Module not found in JSON: " + name);

		_ModuleBase *pM = static_cast<_ModuleBase *>(pModule);
		pM->setName(name);
		m_vModules.push_back(pM);
		LOG_I("Added: " + name);

		return true;
	}

	bool ModuleMgr::bStdErr(void)
	{
		const json *pJ = findJson("APP");
		IF__(!pJ, true);

		bool bStdErr = true;
		jKv(*pJ, "bStdErr", bStdErr);
		return bStdErr;
	}

	string ModuleMgr::getName(void)
	{
		return m_name;
	}

}
