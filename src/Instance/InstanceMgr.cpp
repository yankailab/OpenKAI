#include "InstanceMgr.h"
#include "Modules.h"
#include "DataStreams.h"

namespace kai
{

	InstanceMgr::InstanceMgr(void)
	{
	}

	InstanceMgr::~InstanceMgr(void)
	{
		cleanAll();
	}

	bool InstanceMgr::loadJsonFiles(const string &fName)
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

	bool InstanceMgr::createAll(void)
	{
		for (size_t i = 0; i < m_vJcfg.size(); i++)
		{
			JsonCfg *pJc = &m_vJcfg[i];
			json *pJs = pJc->getJson();

			for (auto it = pJs->begin(); it != pJs->end(); it++)
			{
				json *pJ = &it.value();
				IF_CONT(!pJ->is_object());

				string n = it.key();
				if (n.empty())
				{
					LOG_E("Module name is empty");
					continue;
				}

				bool bON = true;
				jKv(*pJ, "bON", bON);
				if (!bON)
				{
					LOG_I("Module disabled: " + n);
					continue;
				}

				string t = "module";
				jKv(*pJ, "type", t);

				if (t == "module")
				{
					addModule(n, pJc, pJ);
				}
				else if (t == "dataStream")
				{
					addDataStream(n, pJc, pJ);
				}
			}
		}

		return true;
	}

	bool InstanceMgr::initAll(void)
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

	bool InstanceMgr::linkAll(void)
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

	bool InstanceMgr::startAll(void)
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

	void InstanceMgr::pauseAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			pM->pause();
		}
	}

	void InstanceMgr::resumeAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			pM->resume();
		}
	}

	void InstanceMgr::stopAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			pM->stop();
		}

		// TODO
	}

	void InstanceMgr::waitForComplete(void)
	{
		// TODO: temporal impl
		while (!bComplete())
		{
			sleep(1);
		}
	}

	bool InstanceMgr::bComplete(void)
	{
		// TODO: temporal impl
		return false;
	}

	void InstanceMgr::cleanAll(void)
	{
		for (_ModuleBase *pM : m_vModules)
		{
			DEL(pM);
		}
		m_vModules.clear();

		for (DataStreamBase *pD : m_vDataStreams)
		{
			DEL(pD);
		}
		m_vDataStreams.clear();
	}

	bool InstanceMgr::addModule(const string &name, JsonCfg *pJc, json *pJ)
	{
		IF_Le_F(findModule(name), "Module already existed: " + name);

		string c = "";
		jKv(*pJ, "class", c);

		IF_Le_F(c.empty(), "Class name is empty: " + name);

		Modules md;
		_ModuleBase *pM = md.createInstance(c);
		IF_Le_F(pM == nullptr, "Instance not created: " + name);

		pM->setName(name);
		pM->setConfig(pJc, pJ);
		m_vModules.push_back(pM);

		LOG_I("Module instance created: " + name);
		return true;
	}

	bool InstanceMgr::addDataStream(const string &name, JsonCfg *pJc, json *pJ)
	{
		IF_Le_F(findDataStream(name), "Data stream already existed: " + name);

		string c = "";
		jKv(*pJ, "class", c);

		IF_Le_F(c.empty(), "Class name is empty: " + name);

		DataStreams ds;
		DataStreamBase *pD = ds.createInstance(c);
		IF_Le_F(pD == nullptr, "Data stream not created: " + name);

		pD->setName(name);
		pD->setConfig(pJc, pJ);
		m_vDataStreams.push_back(pD);

		LOG_I("Data stream instance created: " + name);
		return true;
	}

	void *InstanceMgr::findModule(const string &name)
	{
		IF_N(name.empty());

		for (_ModuleBase *pM : m_vModules)
		{
			IF__(name == pM->getName(), pM);
		}

		return nullptr;
	}

	void *InstanceMgr::findDataStream(const string &name)
	{
		IF_N(name.empty());

		for (DataStreamBase *pD : m_vDataStreams)
		{
			IF__(name == pD->getName(), pD);
		}

		return nullptr;
	}

	JsonCfg *InstanceMgr::findJsonCfg(const string &name)
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

	json *InstanceMgr::findJson(const string &name)
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

	bool InstanceMgr::addExternalModule(void *pModule, const string &name)
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

	bool InstanceMgr::bStdErr(void)
	{
		const json *pJ = findJson("APP");
		IF__(!pJ, true);

		bool bStdErr = true;
		jKv(*pJ, "bStdErr", bStdErr);
		return bStdErr;
	}

	string InstanceMgr::getName(void)
	{
		return m_name;
	}

}
