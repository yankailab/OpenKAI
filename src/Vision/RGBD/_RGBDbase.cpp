/*
 * _RGBDbase.cpp
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#include "_RGBDbase.h"

namespace kai
{

	_RGBDbase::_RGBDbase()
	{
	}

	_RGBDbase::~_RGBDbase()
	{
		DEL(m_pTpp);
	}

	bool _RGBDbase::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "devFPSd", m_devFPSd);
		jKv<int>(j, "vSizeD", m_vSizeD);
		jKv<float>(j, "vRangeD", m_vRangeD);
		jKv(j, "dScale", m_dScale);
		jKv(j, "dOfs", m_dOfs);

		jKv(j, "bDepth", m_bDepth);
		jKv(j, "bIR", m_bIR);
		jKv(j, "btRGB", m_btRGB);
		jKv(j, "btDepth", m_btDepth);
		jKv(j, "bConfidence", m_bConfidence);
		jKv(j, "fConfidenceThr", m_fConfidenceThr);

		jKv(j, "bIMU", m_bIMU);
		jKv(j, "bPCL", m_bPCL);
		jKv(j, "bPCLrgb", m_bPCLrgb);

		return true;
	}

	bool _RGBDbase::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		j["devFPSd"] = m_devFPSd;
		j["vSizeD"] = {m_vSizeD.x(), m_vSizeD.y()};
		j["vRangeD"] = {m_vRangeD.x(), m_vRangeD.y()};
		j["dScale"] = m_dScale;
		j["dOfs"] = m_dOfs;
		j["bDepth"] = m_bDepth;
		j["bIR"] = m_bIR;
		j["btRGB"] = m_btRGB;
		j["btDepth"] = m_btDepth;
		j["bConfidence"] = m_bConfidence;
		j["fConfidenceThr"] = m_fConfidenceThr;
		j["bIMU"] = m_bIMU;
		j["bPCL"] = m_bPCL;
		j["bPCLrgb"] = m_bPCLrgb;

		IF_F(m_pTpp && !m_pTpp->saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _RGBDbase::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n;

		n = "";
		jKv(j, "RGBDframe", n);
		m_pRGBD = dynamic_cast<RGBDframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pRGBD, "Data stream not found: " + n);

		n = "";
		jKv(j, "RGBDtRGBframe", n);
		m_pRGBDtRGB = dynamic_cast<RGBDframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pRGBDtRGB, "Data stream not found: " + n);

		n = "";
		jKv(j, "RGBDtDframe", n);
		m_pRGBDtD = dynamic_cast<RGBDframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pRGBDtD, "Data stream not found: " + n);

		n = "";
		jKv(j, "Dframe", n);
		m_pD = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pD, "Data stream not found: " + n);

		n = "";
		jKv(j, "IRframe", n);
		m_pIR = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pIR, "Data stream not found: " + n);

		n = "";
		jKv(j, "PCLframe", n);
		m_pPCL = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pPCL, "Data stream not found: " + n);

		n = "";
		jKv(j, "IMUframe", n);
		m_pIMU = dynamic_cast<IMUstream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pIMU, "Data stream not found: " + n);

		return true;
	}

	bool _RGBDbase::check(void)
	{
		return _RGBbase::check();
	}

	Vector2f _RGBDbase::getDepthRange(void)
	{
		return m_vRangeD;
	}

	float _RGBDbase::getDepthScale(void)
	{
		return m_dScale;
	}

	float _RGBDbase::getDepthOffset(void)
	{
		return m_dOfs;
	}

	void _RGBDbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_RGBbase::console(pConsole);

		NULL_(m_pTpp);
        m_pTpp->console(pConsole);
	}


}
