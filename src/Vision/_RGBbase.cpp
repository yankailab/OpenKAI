/*
 * _RGBbase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_RGBbase.h"

namespace kai
{

	_RGBbase::_RGBbase()
	{
	}

	_RGBbase::~_RGBbase()
	{
	}

	bool _RGBbase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "devURI", m_devURI);
		jKv(j, "devFPS", m_devFPS);
		jKv(j, "bRGB", m_bRGB);
		jKv<int>(j, "vSizeRGB", m_vSizeRGB);

		return true;
	}

	bool _RGBbase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["devURI"] = m_devURI;
		j["devFPS"] = m_devFPS;
		j["bRGB"] = m_bRGB;
		j["vSizeRGB"] = {m_vSizeRGB.x(), m_vSizeRGB.y()};

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _RGBbase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeOut", n);
		m_pRGBout = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pRGBout, "RGBframeOut not found: " + n);

		return true;
	}

	bool _RGBbase::open(void)
	{
		return false;
	}

	bool _RGBbase::bOpened(void)
	{
		return m_bOpened;
	}

	void _RGBbase::close(void)
	{
		m_bOpened = false;
	}

	bool _RGBbase::check(void)
	{
		return _ModuleBase::check();
	}

	void _RGBbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

	void _RGBbase::console(const json &j, void *pJSONbase)
	{
		string cmd;
		IF_(!jKv(j, "cmd", cmd));
	}

}
