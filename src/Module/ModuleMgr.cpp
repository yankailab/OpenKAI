#include "ModuleMgr.h"
#include "Module.h"
#include "../Base/BASE.h"

namespace kai
{

	ModuleMgr::ModuleMgr(void)
	{
		m_pJcfg = nullptr;
	}

	ModuleMgr::~ModuleMgr(void)
	{
		for (void *pM : m_vModules)
		{
			DEL(pM);
		}

		m_vModules.clear();
	}

	bool ModuleMgr::addJsonCfg(JsonCfg *pJcfg)
	{
	}

	string ModuleMgr::getName(void)
	{
		return m_name;
	}

	bool ModuleMgr::createAll(void)
	{
		NULL_F(m_pJcfg);

		Module md;
		json js = m_pJcfg->getJson();
		for (auto it = js.begin(); it != js.end(); it++)
		{
			const json &j = it.value();
			IF_CONT(!j.is_object());

			string n = j.value("name", "");
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

			string c = j.value("class", "");
			IF_CONT(c == "ModuleMgr");
			if (c.empty())
			{
				LOG_I("Class name is empty: " + n);
				continue;
			}

			int bON = j.value("bON", 1);
			if (bON == 0)
			{
				LOG_I("Module disabled: " + n);
				continue;
			}

			BASE *pM = md.createInstance(c);
			if (pM == nullptr)
			{
				LOG_I("Failed to create instance: " + n);
				continue;
			}

//			pM->set m_name

			m_vModules.push_back((void*)pM);
		}

		return true;
	}

	bool ModuleMgr::initAll(void)
	{
		for (void* pM : m_vModules)
		{
			BASE* pB = (BASE*)pM;

// TODO assign json ptr for each module

			// if (pB->init(pK) != OK_OK)
			// {
			// 	LOG_E(pB->getName() + ".init()");
			// }
		}

		return true;
	}

	bool ModuleMgr::linkAll(void)
	{
		for (void* pM : m_vModules)
		{
			BASE* pB = (BASE*)pM;
			if (pB->link() != OK_OK)
			{
				LOG_E(pB->getName() + ".link()");
			}
		}

		return true;
	}

	bool ModuleMgr::startAll(void)
	{
		for (void* pM : m_vModules)
		{
			BASE* pB = (BASE*)pM;
			if (pB->start() != OK_OK)
			{
				LOG_E(pB->getName() + ".start()");
			}
		}

		return true;
	}

	void ModuleMgr::resumeAll(void)
	{
		for (void* pM : m_vModules)
		{
			((BASE*)pM)->resume();
		}
	}

	void ModuleMgr::pauseAll(void)
	{
		for (void* pM : m_vModules)
		{
			((BASE*)pM)->pause();
		}
	}

	void ModuleMgr::stopAll(void)
	{
		for (void* pM : m_vModules)
		{
			((BASE*)pM)->stop();
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

	}

}
