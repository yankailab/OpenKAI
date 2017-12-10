/*
 *  Created on: Nov 24, 2017
 *      Author: yankai
 */
#include "_RPLIDAR.h"

namespace kai
{

_RPLIDAR::_RPLIDAR()
{
	m_nDiv = 0;
	m_dAngle = 0;
	m_dMin = 0.1;
	m_dMax = 100.0;
	m_angleV = 0.0;
	m_angleH = 0.0;

	m_portName = "";
	m_baud = 115200;

	m_bReady = 0;
	m_nDetection = 0;
	m_timeStamp = 0;
}

_RPLIDAR::~_RPLIDAR()
{

}

bool _RPLIDAR::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("minDist", &m_dMin));
	F_INFO(pK->v("maxDist", &m_dMax));
	F_INFO(pK->v("portName", &m_portName));
	F_INFO(pK->v("baud", &m_baud));

	return true;
}

bool _RPLIDAR::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

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
		if (!open())
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		this->autoFPSfrom();

		updateLidar();

		this->autoFPSto();
	}
}

bool _RPLIDAR::open(void)
{
	u_result op_result;

	// create the driver instance
	RPlidarDriver * drv = RPlidarDriver::CreateDriver(
			RPlidarDriver::DRIVER_TYPE_SERIALPORT);

	if (!drv)
	{
		fprintf(stderr, "insufficent memory, exit\n");
		exit(-2);
	}

	// make connection...
	if (IS_FAIL(drv->connect(m_portName.c_str(), m_baud)))
	{
		LOG_E("Error, cannot bind to the specified serial port");
		goto on_finished;
	}

	rplidar_response_device_info_t devinfo;

	// retrieving the device info
	////////////////////////////////////////
	op_result = drv->getDeviceInfo(devinfo);

	if (IS_FAIL(op_result))
	{
		fprintf(stderr, "Error, cannot get device info.\n");
		goto on_finished;
	}

	// print out the device serial number, firmware and hardware version number..
	printf("RPLIDAR S/N: ");
	for (int pos = 0; pos < 16; ++pos)
	{
		printf("%02X", devinfo.serialnum[pos]);
	}

	printf("\n"
			"Firmware Ver: %d.%02d\n"
			"Hardware Rev: %d\n", devinfo.firmware_version >> 8,
			devinfo.firmware_version & 0xFF, (int) devinfo.hardware_version);

	// check health...
	if (!checkRPLIDARHealth(drv))
	{
		goto on_finished;
	}

	drv->startMotor();
	// start scan...
	drv->startScan();

	// fetech result and print it out...
	while (1)
	{
		rplidar_response_measurement_node_t nodes[360 * 2];
		size_t count = _countof(nodes);

		op_result = drv->grabScanData(nodes, count);

		if (IS_OK(op_result))
		{
			drv->ascendScanData(nodes, count);
			for (int pos = 0; pos < (int) count; ++pos)
			{
				printf("%s theta: %03.2f Dist: %08.2f Q: %d \n",
						(nodes[pos].sync_quality
								& RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ?
								"S " : "  ",
						(nodes[pos].angle_q6_checkbit
								>> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)
								/ 64.0f, nodes[pos].distance_q2 / 4.0f,
						nodes[pos].sync_quality
								>> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);
			}
		}

	}

	drv->stop();
	drv->stopMotor();
	// done!
	on_finished: RPlidarDriver::DisposeDriver(drv);

	return true;
}

bool _RPLIDAR::checkRPLIDARHealth(RPlidarDriver * drv)
{
    u_result     op_result;
    rplidar_response_device_health_t healthinfo;


    op_result = drv->getHealth(healthinfo);
    if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        printf("RPLidar health status : %d\n", healthinfo.status);
        if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
            fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // drv->reset();
            return false;
        } else {
            return true;
        }

    } else {
        fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
        return false;
    }
}

bool _RPLIDAR::updateLidar(void)
{
	return true;
}

DIST_SENSOR_TYPE _RPLIDAR::type(void)
{
	return dsRPLIDAR;
}

vDouble2 _RPLIDAR::range(void)
{
	vDouble2 r;
	r.x = m_dMin;
	r.y = m_dMax;
	return r;
}

bool _RPLIDAR::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	pWin->tabNext();
	msg = "";
	pWin->addMsg(&msg);

	pWin->tabPrev();
	IF_T(m_nDiv <= 0);

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	for (int i = 0; i < m_nDiv; i++)
	{
	}

	return true;
}

}
