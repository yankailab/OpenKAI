/*
 * _HiphenCMD.cpp
 *
 *  Created on: Feb 19, 2019
 *      Author: yankai
 */

#include "_HiphenCMD.h"

namespace kai
{

_HiphenCMD::_HiphenCMD()
{
    m_pTr = NULL;
}

_HiphenCMD::~_HiphenCMD()
{
    DEL ( m_pTr );
}

bool _HiphenCMD::init(void* pKiss)
{
	IF_F(!_TCPclient::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
    
    Kiss* pKt = pK->child("threadR");
    IF_F(pKt->empty());
    
    m_pTr = new _Thread();
    if(!m_pTr->init(pKt))
    {
        DEL(m_pTr);
        return false;
    }

	return true;
}

bool _HiphenCMD::start(void)
{
    NULL_F(m_pT);
    NULL_F(m_pTr);
	IF_F(!m_pT->start(getUpdateW, this));
	return m_pTr->start(getUpdateR, this);
}

void _HiphenCMD::updateW(void)
{
	while(m_pT->bRun())
	{
		if (!isOpen())
		{
			if (!open())
			{
				m_pT->sleepT (USEC_1SEC);
				continue;
			}

			this->startRecord();
		}

		m_pT->autoFPSfrom();

		uint8_t pB[N_IO_BUF];
		int nB;
		while((nB = m_fifoW.output(pB, N_IO_BUF)) > 0)
		{
			int nSend = ::send(m_socket, pB, nB, 0);
			if (nSend == -1)
			{
				if(errno == EAGAIN)break;
				if(errno == EWOULDBLOCK)break;
				LOG_E("send error: " + i2str(errno));
				close();
				break;
			}

			LOG_I("send: " + i2str(nSend) + " bytes");
		}

		m_pT->autoFPSto();
	}

}

void _HiphenCMD::updateR(void)
{
	while (m_pTr->bRun())
	{
		if (!isOpen())
		{
			::sleep(1);
			continue;
		}

		uint8_t pB[N_IO_BUF];
		int nR = ::recv(m_socket, pB, N_IO_BUF, 0);
		if (nR <= 0)
		{
			LOG_E("recv error: " + i2str(errno));
			close();
			continue;
		}

		pB[nR]=0;
		string strR((char*)pB);
		LOG_I("Received: " + strR);
		//TODO: add JSON decode for strR;
	}
}

void _HiphenCMD::getID(void)
{
	const string cmd = "{\"command\":\"get_id\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getStatus(void)
{
	const string cmd = "{\"command\":\"get_status\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getTime(void)
{
	const string cmd = "{\"command\":\"get_time\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getBatteryLevel(void)
{
	const string cmd = "{\"command\":\"get_battery_level\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getGPScoordinates(void)
{
	const string cmd = "{\"command\":\"get_gps_coordinates\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getImageInfo(void)
{
	const string cmd = "{\"command\":\"get_image_info\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::setImageInfo(string& opr, string& imgDesc)
{
	string cmd = "{\"command\":\"set_image_info\",\"operator\":\""
			+ opr
			+"\",\"image_description\":\"" + imgDesc
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getTriggerSettings(void)
{
	const string cmd = "{\"command\":\"get_trigger_settings\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::setTriggerSettings(string& tIn, string& tOut)
{
	string cmd = "{\"command\":\"set_trigger_settings\",\"trigger_in\":\""
			+ tIn
			+"\",\"trigger_out\":\"" + tOut
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getExposureMode(void)
{
	const string cmd = "{\"command\":\"get_exposure_mode\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::setExposureMode(string& mode)
{
	string cmd = "{\"command\":\"set_exposure_mode\",\"exposure_mode\":\""
			+ mode
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getIntegrationTime(int iSensor)
{
	string cmd = "{\"command\":\"get_integration_time\",\"sensor\":\""
			+ i2str(iSensor)
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::setIntegrationTime(int iSensor, int tInt)
{
	string cmd = "{\"command\":\"set_integration_time\",\"sensor\":\""
			+ i2str(iSensor)
			+"\",\"integration_time\":\"" + i2str(tInt)
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getGainMode(void)
{
	const string cmd = "{\"command\":\"get_gain_mode\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::setGainMode(string& mode)
{
	string cmd = "{\"command\":\"set_gain_time\",\"gain_mode\":\""
			+ mode
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::getGain(int iSensor)
{
	string cmd = "{\"command\":\"get_gain\",\"sensor\":\""
			+ i2str(iSensor)
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::setGain(int iSensor, int gain)
{
	string cmd = "{\"command\":\"set_gain\",\"sensor\":\""
			+ i2str(iSensor)
			+"\",\"gain\":\"" + i2str(gain)
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::startRecord(void)
{
	const string cmd = "{\"command\":\"start_record\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::stopRecord(void)
{
	const string cmd = "{\"command\":\"stop_record\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

void _HiphenCMD::snapshot(int nSnapshot, string& mode)
{
	string cmd = "{\"command\":\"snapshot\",\"snapshot_number\":\""
			+ i2str(nSnapshot)
			+"\",\"snapshot_mode\":\"" + mode
			+"\"}";
	write((uint8_t*)cmd.c_str(), cmd.length());
}

}
