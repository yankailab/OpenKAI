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
		for (void *pM : m_vModules)
		{
			DEL(pM);
		}

		m_vModules.clear();
	}

	bool ModuleMgr::parseJsonFile(const string &fName)
	{
		return m_jCfg.parseJsonFile(fName);
	}

	void ModuleMgr::setJsonCfg(const JsonCfg &jCfg)
	{
		m_jCfg = jCfg;
	}

	string ModuleMgr::getName(void)
	{
		return m_name;
	}

	bool ModuleMgr::createAll(void)
	{
		Module md;
		json jC = m_jCfg.getJson();
		for (auto it = jC.begin(); it != jC.end(); it++)
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

			string c = Ji.value("class", "");
			IF_CONT(c == "ModuleMgr");
			if (c.empty())
			{
				LOG_I("Class name is empty: " + n);
				continue;
			}

			int bON = Ji.value("bON", 1);
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
		}

		return true;
	}

	bool ModuleMgr::initAll(void)
	{
		const json &j = m_jCfg.getJson();

		for (void *pM : m_vModules)
		{
			BASE *pB = (BASE *)pM;

			const json &jm = j.at(pB->getName());
			if (!jm.is_object())
			{
				LOG_E(pB->getName() + " is not an JSON object");
				return false;
			}

			if (!pB->init(jm))
			{
				LOG_E(pB->getName() + ".init() failed");
				return false;
			}
		}

		return true;
	}

	bool ModuleMgr::linkAll(void)
	{
		const json &j = m_jCfg.getJson();

		for (void *pM : m_vModules)
		{
			BASE *pB = (BASE *)pM;

			const json &jm = j.at(pB->getName());
			if (!jm.is_object())
			{
				LOG_E(pB->getName() + " is not an json object");
				return false;
			}

			if (!pB->link(jm, this))
			{
				LOG_E(pB->getName() + ".link() failed");
				return false;
			}
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

}
