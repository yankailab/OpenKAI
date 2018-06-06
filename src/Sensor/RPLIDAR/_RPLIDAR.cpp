/*
 *  Created on: Nov 24, 2017
 *      Author: yankai
 */
#include "_RPLIDAR.h"

namespace kai
{

_RPLIDAR::_RPLIDAR()
{
	m_pRPL = NULL;
	m_portName = "";
	m_baud = 115200;
}

_RPLIDAR::~_RPLIDAR()
{
	close();
}

bool _RPLIDAR::init(void* pKiss)
{
	IF_F(!this->_DistSensorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,portName);
	KISSm(pK,baud);

	return true;
}

bool _RPLIDAR::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _RPLIDAR::update(void)
{
	while (m_bThreadON)
	{
		if(!m_pRPL)
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		updateLidar();

		this->autoFPSto();
	}
}

bool _RPLIDAR::open(void)
{
	m_pRPL = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);
	NULL_F(m_pRPL);

	if (IS_FAIL(m_pRPL->connect(m_portName.c_str(), m_baud)))
	{
		LOG_E("Cannot bind to the serial port");
		RPlidarDriver::DisposeDriver(m_pRPL);
		m_pRPL = NULL;
		return false;
	}

	u_result rpResult;
	rplidar_response_device_info_t devinfo;
	rpResult = m_pRPL->getDeviceInfo(devinfo);
	if (IS_FAIL(rpResult))
	{
		LOG_E("Cannot get device info");
		RPlidarDriver::DisposeDriver(m_pRPL);
		m_pRPL = NULL;
		return false;
	}

	string strRP = "RPLIDAR S/N:";
	char pBuf[128];
	for (int pos = 0; pos < 16; ++pos)
	{
		sprintf(&pBuf[pos], "%02X", devinfo.serialnum[pos]);
	}
	strRP += pBuf;

	sprintf(pBuf, "; Firmware Ver: %d.%02d; Hardware Rev: %d;",
			devinfo.firmware_version >> 8, devinfo.firmware_version & 0xFF,
			(int) devinfo.hardware_version);
	strRP += pBuf;

	LOG_I(strRP);

	if (!checkRPLIDARHealth())
	{
		RPlidarDriver::DisposeDriver(m_pRPL);
		m_pRPL = NULL;
	}

	m_pRPL->startMotor();
	m_pRPL->startScan();

	m_bReady = true;

	return true;
}

void _RPLIDAR::close(void)
{
	NULL_(m_pRPL);

	m_pRPL->stop();
	m_pRPL->stopMotor();
	RPlidarDriver::DisposeDriver(m_pRPL);
	m_pRPL = NULL;
}

bool _RPLIDAR::checkRPLIDARHealth(void)
{
	NULL_F(m_pRPL);
	u_result rpResult;
	rplidar_response_device_health_t healthinfo;

	rpResult = m_pRPL->getHealth(healthinfo);
	if (!IS_OK(rpResult))
	{
		LOG_E("Cannot retrieve the RPLIDAR health code: " + i2str(rpResult));
		return false;
	}

	// the macro IS_OK is the preperred way to judge whether the operation is succeed.
	LOG_I("RPLidar health status : " + i2str(healthinfo.status));

	if (healthinfo.status == RPLIDAR_STATUS_ERROR)
	{
		LOG_E("RPLIDAR internal error detected. Rebooting the device");
		m_pRPL->reset();
		return false;
	}

	return true;
}

void _RPLIDAR::updateLidar(void)
{
	u_result rpResult;

	rplidar_response_measurement_node_t nodes[360 * 2];
	size_t count = _countof(nodes);

	rpResult = m_pRPL->grabScanData(nodes, count);
	IF_(!IS_OK(rpResult));

	m_pRPL->ascendScanData(nodes, count);
	for (int i=0; i < (int) count; i++)
	{
		double angle = (nodes[i].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) * 0.015625f;// / 64.0f;
		double d = nodes[i].distance_q2 * 0.00025f;  // /(4.0*1000.0)
		this->input(angle, d, 0);

		LOG_I("Angle:" + f2str(angle) + " D:" + f2str(d));

//		printf("%s theta: %03.2f Dist: %08.2f Q: %d \n",
//				(nodes[i].sync_quality & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ? "S " : "  ",
//				(nodes[i].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f,
//				nodes[i].distance_q2 / 4.0f,
//				nodes[i].sync_quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);
	}
}

DIST_SENSOR_TYPE _RPLIDAR::type(void)
{
	return dsRPLIDAR;
}


}
