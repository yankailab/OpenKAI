#include "ModuleMgr.h"
#include "Module.h"
#include "../Base/BASE.h"

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
		// Existing modules retain pointers into their owning configuration files.
		if (!m_vModules.empty())
		{
			LOG_E("Cannot reload JSON files while modules exist");
			return false;
		}

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
			const json &J = *pJc->getJson();
			for (auto it = J.begin(); it != J.end(); it++)
			{
				const json &Ji = it.value();
				IF_CONT(!Ji.is_object());

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
				jKv(Ji, "class", c);
				IF_CONT(c == "ModuleMgr");
				if (c.empty())
				{
					LOG_I("Class name is empty: " + n);
					continue;
				}

				int bON = true;
				jKv(Ji, "bON", bON);
				if (bON == 0)
				{
					LOG_I("Module disabled: " + n);
					continue;
				}

				BASE *pB = md.createInstance(c);
				if (pB == nullptr)
				{
					LOG_I("Instance not created: " + n);
					continue;
				}

				pB->setModuleMgr(this);
				pB->setName(n);
				m_vModules.push_back(pB);
				LOG_I("Instance created: " + n);
			}
		}

		return true;
	}

	bool ModuleMgr::initAll(void)
	{
		for (BASE *pB : m_vModules)
		{
			if (!pB->loadConfig())
			{
				LOG_E(pB->getName() + ".loadConfig() failed");
				return false;
			}

			LOG_I("Initialized: " + pB->getName());
		}

		return true;
	}

	bool ModuleMgr::linkAll(void)
	{
		for (BASE *pB : m_vModules)
		{
			if (!pB->link())
			{
				LOG_E(pB->getName() + ".link() failed");
				return false;
			}

			LOG_I("Linked: " + pB->getName());
		}

		return true;
	}

	bool ModuleMgr::startAll(void)
	{
		for (BASE *pB : m_vModules)
		{
			if (!pB->start())
			{
				LOG_E(pB->getName() + ".start() failed");
				return false;
			}

			LOG_I("Started: " + pB->getName());
		}

		return true;
	}

	void ModuleMgr::pauseAll(void)
	{
		for (BASE *pM : m_vModules)
		{
			pM->pause();
		}
	}

	void ModuleMgr::resumeAll(void)
	{
		for (BASE *pM : m_vModules)
		{
			pM->resume();
		}
	}

	void ModuleMgr::stopAll(void)
	{
		for (BASE *pM : m_vModules)
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
		for (BASE *pM : m_vModules)
		{
			DEL(pM);
		}

		m_vModules.clear();
	}

	void *ModuleMgr::findModule(const string &name)
	{
		IF_N(name.empty());

		for (BASE *pM : m_vModules)
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

		BASE *pB = static_cast<BASE *>(pModule);
		pB->setModuleMgr(this);
		pB->setName(name);
		m_vModules.push_back(pB);
		LOG_I("Added: " + name);

		return true;
	}

	bool ModuleMgr::bStdErr(void)
	{
		const json *pJ = findJson("APP");
		if (!pJ)
		{
			return true;
		}

		bool bStdErr = true;
		jKv(*pJ, "bStdErr", bStdErr);
		return bStdErr;
	}

	string ModuleMgr::getName(void)
	{
		return m_name;
	}

}
