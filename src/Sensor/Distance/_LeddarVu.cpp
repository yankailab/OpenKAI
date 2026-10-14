/*
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */
#include "_LeddarVu.h"
#include "DistancePoint.h"
#include "../../UI/_Console.h"

namespace kai
{

	_LeddarVu::_LeddarVu()
	{
	}

	_LeddarVu::~_LeddarVu()
	{
		if (m_pT)
		{
			m_pT->join();
		}
		if (m_pMb)
		{
			modbus_close(m_pMb);
			modbus_free(m_pMb);
		}
	}

	bool _LeddarVu::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "nDiv", m_nDiv);
		IF_F(m_nDiv <= 0 || m_nDiv > LEDDAR_MAX_DETECTIONS);
		jKv(j, "fovH", m_fovH);
		IF_F(!std::isfinite(m_fovH) || m_fovH <= 0 || m_fovH > 360);
		jKv<float>(j, "vRange", m_vRange);
		IF_F(!m_vRange.allFinite() || m_vRange.x() < 0 || m_vRange.x() >= m_vRange.y());

		jKv(j, "port", m_port);
		jKv(j, "baud", m_baud);
		jKv(j, "slaveAddr", m_slaveAddr);
		jKv(j, "bUse0x41", m_bUse0x41);

		jKv(j, "nAccumulationsExpo", m_nAccumulationsExpo);
		jKv(j, "nOversamplingsExpo", m_nOversamplingsExpo);
		jKv(j, "nPoint", m_nPoint);
		jKv(j, "lightSrcPwr", m_lightSrcPwr);
		jKv(j, "bAutoLightSrcPwr", m_bAutoLightSrcPwr);
		jKv(j, "bDemergeObj", m_bDemergeObj);
		jKv(j, "bStaticNoiseRemoval", m_bStaticNoiseRemoval);
		jKv(j, "bPrecicion", m_bPrecision);
		jKv(j, "bSaturationCompensation", m_bSaturationCompensation);
		jKv(j, "bOvershootManagement", m_bOvershootManagement);
		jKv(j, "oprMode", m_oprMode);

		return true;
	}

	bool _LeddarVu::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["nDiv"] = m_nDiv;
		j["fovH"] = m_fovH;
		j["vRange"] = {m_vRange.x(), m_vRange.y()};
		j["port"] = m_port;
		j["baud"] = m_baud;
		j["slaveAddr"] = m_slaveAddr;
		j["bUse0x41"] = m_bUse0x41;
		j["nAccumulationsExpo"] = m_nAccumulationsExpo;
		j["nOversamplingsExpo"] = m_nOversamplingsExpo;
		j["nPoint"] = m_nPoint;
		j["lightSrcPwr"] = m_lightSrcPwr;
		j["bAutoLightSrcPwr"] = m_bAutoLightSrcPwr;
		j["bDemergeObj"] = m_bDemergeObj;
		j["bStaticNoiseRemoval"] = m_bStaticNoiseRemoval;
		j["bPrecicion"] = m_bPrecision;
		j["bSaturationCompensation"] = m_bSaturationCompensation;
		j["bOvershootManagement"] = m_bOvershootManagement;
		j["oprMode"] = m_oprMode;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _LeddarVu::link(InstanceMgr *pM)
	{
		IF_F(!_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n;
		jKv(j, "PCLframeOut", n);
		m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + n);

		return true;
	}

	bool _LeddarVu::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _LeddarVu::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_pMb)
			{
				if (!open())
				{
					m_pT->sleepT(NSEC_SEC);
					continue;
				}
			}

			m_pT->autoFPS();

			if (m_bUse0x41)
			{
				updateLidarFast();
			}
			else
			{
				updateLidar();
			}
		}
	}

	bool _LeddarVu::open(void)
	{
		m_pMb = modbus_new_rtu(m_port.c_str(), m_baud, 'N', 8, 1);
		if (m_pMb == nullptr)
		{
			m_pMb = NULL;
			LOG_E("Cannot create the libmodbus context");
			return false;
		}

		modbus_set_debug(m_pMb, false);

		if (modbus_connect(m_pMb) != 0)
		{
			LOG_E("Modbus connection failed");
			modbus_free(m_pMb);
			m_pMb = NULL;
			return false;
		}

		if (modbus_set_slave(m_pMb, m_slaveAddr) != 0)
		{
			LOG_E("Error setting slave id");
			m_slaveAddr = -1;
			return false;
		}

		// We send a MODBUS_SERVER_ID request to determine if we are talking to a Leddar sensor.
		uint8_t buf[256];
		buf[0] = 1;
		buf[1] = 0x11; // Request MODBUS_SERVER_ID

		if (modbus_send_raw_request(m_pMb, buf, 2) < 2)
		{
			LOG_E("Error sending MODBUS_SERVER_ID request");
			return false;
		}

		memset(buf, 0, sizeof buf);
		int nReceived = modbus_receive_confirmation(m_pMb, buf);
		if (nReceived < 0)
		{
			LOG_E("Error receiving data from the sensor: " + i2str(errno));
			return false;
		}

		LOG_I("Received " + i2str(nReceived) + " bytes from the sensor");

		if (nReceived < 155)
		{
			LOG_E("Unexpected answer");
			return false;
		}

		if (!((buf[0] == 1) && (buf[1] == 0x11)))
		{
			LOG_E("Bad address or function");
			return false;
		}

		if ((buf[2 + 150] == 9) && (buf[2 + 151] == 0))
		{
			LOG_I("LeddarTech M16");
		}
		else if ((buf[2 + 150] == 7) && (buf[2 + 151] == 0))
		{
			LOG_I("LeddarTech evaluation kit");
		}
		else if ((buf[2 + 50] == 10) && (buf[2 + 51] == 0))
		{
			LOG_I("LeddarOne (please use the LeddarOne sample)");
		}
		else
		{
			string str = "";
			for (int i = 0; i < nReceived; i++)
				str += (buf[i] >= 32) ? buf[i] : ' ';

			string::size_type iSensor = str.find("Sensor");
			if (iSensor != string::npos)
			{
				str = str.substr(iSensor, 20);
			}

			LOG_I(str);
		}

		modbus_write_register(m_pMb, 0, m_nAccumulationsExpo);
		modbus_write_register(m_pMb, 1, m_nOversamplingsExpo);
		modbus_write_register(m_pMb, 2, m_nPoint);
		modbus_write_register(m_pMb, 5, m_lightSrcPwr);
		modbus_write_register(m_pMb, 10, m_oprMode);

		uint16_t acOpt = 0;
		if (m_bAutoLightSrcPwr)
			acOpt |= 1;
		if (m_bDemergeObj)
			acOpt |= 1 << 1;
		if (m_bStaticNoiseRemoval)
			acOpt |= 1 << 2;
		if (m_bPrecision)
			acOpt |= 1 << 3;
		if (m_bSaturationCompensation)
			acOpt |= 1 << 4;
		if (m_bOvershootManagement)
			acOpt |= 1 << 5;
		modbus_write_register(m_pMb, 6, acOpt);

		return true;
	}

	bool _LeddarVu::updateLidar(void)
	{
		IF_F(!m_pMb || !m_pPCLout);

		const int nRegisters = 15 + m_nDiv * 2;
		vector<uint16_t> reg(nRegisters);
		int nRead = modbus_read_input_registers(m_pMb, 1, nRegisters, reg.data());
		IF_F(nRead != nRegisters || reg[0] == 0);
		// The response layout must match the configured division count.
		IF_F(reg[1] != m_nDiv);

		m_nDetection = reg[10];
		m_lightSrcPwr = reg[11];
		uint64_t tStamp = getTns();
		vector<GEOMETRY_POINT> points(m_nDiv);
		for (int i = 0; i < m_nDiv; i++)
		{
			float d = reg[15 + i] * 0.01f;
			float a = reg[15 + m_nDiv + i] / 64.0f;
			points[i] = distancePoint(d, a, i, m_nDiv, m_fovH, m_vRange, tStamp);
		}
		m_pPCLout->set(points, tStamp);
		return true;
	}

	bool _LeddarVu::updateLidarFast(void)
	{
		IF_F(!m_pMb || !m_pPCLout);

		// This version of the ReadDetections function uses the 0x41 function code.
		// It reduces the overhead and is the recommended way of reading measurements.

		// Read 5 holding registers from address 1
		uint8_t rawReq[] =
			{(uint8_t)m_slaveAddr, 0x41};
		uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];

		//	if (modbus_send_raw_request(m_pMb, rawReq, ARRAYSIZE(rawReq)) < 0)
		if (modbus_send_raw_request(m_pMb, rawReq, 2) < 0)
		{
			LOG_E("Error sending command 0x41, errno = " + i2str(errno));
			return false;
		}

		// We had to add support for 0x41 in 'compute_data_length_after_meta()' in "modbus.c".
		int iResponseLength = modbus_receive_confirmation(m_pMb, rsp);
		if (iResponseLength < 3)
		{
			LOG_E("Error receiving response for command 0x41, length = " + i2str(iResponseLength) + "; errno = " + i2str(errno));
			return false;
		}

		const int nDetection = rsp[2];
		IF_F(nDetection > LEDDAR_MAX_DETECTIONS || iResponseLength < 3 + nDetection * 5);

		uint64_t tStamp = getTns();
		vector<GEOMETRY_POINT> points(m_nDiv);
		for (int i = 0; i < nDetection; i++)
		{
			const uint8_t *pDetection = rsp + 3 + 5 * i;
			int iDiv = pDetection[4] >> 4;
			IF_CONT(iDiv >= m_nDiv);

			float d = (pDetection[0] | (pDetection[1] << 8)) * 0.01f;
			float a = (pDetection[2] | (pDetection[3] << 8)) / 64.0f;
			GEOMETRY_POINT point = distancePoint(d, a, iDiv, m_nDiv, m_fovH, m_vRange, tStamp);
			// Retain the nearest valid return when a channel has multiple detections.
			if (!points[iDiv].m_tStamp || (point.m_tStamp &&
				point.m_vP.squaredNorm() < points[iDiv].m_vP.squaredNorm()))
			{
				points[iDiv] = point;
			}
		}
		m_nDetection = nDetection;
		m_pPCLout->set(points, tStamp);
		return true;
	}

	void _LeddarVu::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		string msg;
		msg += "nDiv=" + i2str(m_nDiv);
		msg += ", nDet=" + i2str(m_nDetection);
		msg += ", lightSrcPwr=" + i2str(m_lightSrcPwr);
		msg += ", tStamp=" + i2str((int)m_pT->getTfromNs());
		pC->addMsg(msg);

		NULL_(m_pPCLout);
		vector<GEOMETRY_POINT> points;
		m_pPCLout->get(points);
		msg = "";
		for (const GEOMETRY_POINT &point : points)
		{
			float d = point.m_tStamp ? point.m_vP.norm() : -1;
			msg += " | " + f2str(d) + " (" + f2str(point.m_vC.x()) + ")";
		}
		pC->addMsg(msg + " |");
	}

}
