#include "ModuleMgr.h"
#include "Module.h"
#include "../Base/BASE.h"

namespace kai
{

	ModuleMgr::ModuleMgr(void)
	{
		m_name = "ModuleMgr";
		m_bLog = true;
		m_jNull = nullptr;
	}

	ModuleMgr::~ModuleMgr(void)
	{
		for (void *pM : m_vModules)
		{
			DEL(pM);
		}

		m_vModules.clear();
	}

	bool ModuleMgr::parseJsonFile(const string &fName)
	{
		JsonCfg jCfg;
		IF_F(!jCfg.parseJsonFile(fName));

		m_vJcfg.clear();
		m_vJcfg.push_back(jCfg);

		const json& jApp = jK(jCfg.getJson(), "APP");
		IF__(!jApp.is_object(), true);

		vector<string> vJsonFiles;
		jKv(jApp, "vInclude", vJsonFiles);
		for (string f : vJsonFiles)
		{
			JsonCfg jCi;
			IF_CONT(!jCi.parseJsonFile(f));

			m_vJcfg.push_back(jCi);
		}

		return true;
	}

	string ModuleMgr::getName(void)
	{
		return m_name;
	}

	bool ModuleMgr::createAll(void)
	{
		Module md;

		for (int i = 0; i < m_vJcfg.size(); i++)
		{
			JsonCfg *pJc = &m_vJcfg[i];
			const json& J = pJc->getJson();
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

				BASE *pM = md.createInstance(c);
				if (pM == nullptr)
				{
					LOG_I("Instance not created: " + n);
					continue;
				}

				pM->setName(n);
				m_vModules.push_back((void *)pM);
				LOG_I("Instance created: " + n);
			}
		}

		return true;
	}

	bool ModuleMgr::initAll(void)
	{
		for (void *pM : m_vModules)
		{
			BASE *pB = (BASE *)pM;

			const json &j = findJson(pB->getName());
			if (!j.is_object())
			{
				LOG_E(pB->getName() + " is not an JSON object");
				return false;
			}

			if (!pB->init(j))
			{
				LOG_E(pB->getName() + ".init() failed");
				return false;
			}

			LOG_I("Initialized: " + pB->getName());
		}

		return true;
	}

	bool ModuleMgr::linkAll(void)
	{
		for (void *pM : m_vModules)
		{
			BASE *pB = (BASE *)pM;

			const json &j = findJson(pB->getName());
			if (!j.is_object())
			{
				LOG_E(pB->getName() + " is not an json object");
				return false;
			}

			if (!pB->link(j, this))
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
		for (void *pM : m_vModules)
		{
			BASE *pB = (BASE *)pM;
			if (!pB->start())
			{
				LOG_E(pB->getName() + ".start() failed");
				return false;
			}

			LOG_I("Started: " + pB->getName());
		}

		return true;
	}

	void ModuleMgr::resumeAll(void)
	{
		for (void *pM : m_vModules)
		{
			((BASE *)pM)->resume();
		}
	}

	void ModuleMgr::pauseAll(void)
	{
		for (void *pM : m_vModules)
		{
			((BASE *)pM)->pause();
		}
	}

	void ModuleMgr::stopAll(void)
	{
		for (void *pM : m_vModules)
		{
			((BASE *)pM)->stop();
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
		for (void *pM : m_vModules)
		{
			DEL(pM);
		}
	}

	void *ModuleMgr::findModule(const string &name)
	{
		IF_N(name.empty());

		for (void *pM : m_vModules)
		{
			if (name == ((BASE *)pM)->getName())
			{
				return pM;
			}
		}

		return nullptr;
	}

	const json& ModuleMgr::findJson(const string &name)
	{
		for (int i = 0; i < m_vJcfg.size(); i++)
		{
			JsonCfg *pJc = &m_vJcfg[i];
			const json& j = jK(pJc->getJson(), name);

			if (j.is_object())
				return j;
		}

		return m_jNull;
	}

	bool ModuleMgr::addModule(void *pModule, const string &name)
	{
		NULL_F(pModule);
		IF_Le_F(findModule(name), "Module already existed: " + name);
		IF_Le_F(!findJson(name).is_object(), "Module not found in JSON");

		((BASE*)pModule)->setName(name);
		m_vModules.push_back(pModule);
		LOG_I("Added: " + name);

		return true;
	}

}
