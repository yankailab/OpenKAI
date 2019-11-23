/*
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */
#include "_Livox.h"

#ifdef USE_LIVOX

namespace kai
{

/** Connect the broadcast device in list, please input the broadcast code and modify the BROADCAST_CODE_LIST_SIZE. */
/*#define BROADCAST_CODE_LIST_SIZE  3
 int lidar_count = BROADCAST_CODE_LIST_SIZE;
 char broadcast_code_list[kMaxLidarCount][kBroadcastCodeSize] = {
 "000000000000002",
 "000000000000003",
 "000000000000004"
 };*/

static LivoxDevice m_pLivoxDevice[kMaxLidarCount];
static uint32_t m_pLivoxDataRecveiveCount[kMaxLidarCount];
static int m_nLivoxLidar;
static char m_pLivoxBroadcastCodeList[kMaxLidarCount][kBroadcastCodeSize];

static void LivoxGetLidarData(uint8_t handle, LivoxEthPacket *data, uint32_t data_num, void *client_data);
static void OnLivoxCallback(uint8_t status, uint8_t handle, uint8_t response, void *data);
static void OnLivoxStopCallback(uint8_t status, uint8_t handle, uint8_t response,void *data);
static void OnLivoxDeviceInformation(uint8_t status, uint8_t handle, DeviceInformationResponse *ack, void *data);
static void OnLivoxDeviceChange(const DeviceInfo *info, DeviceEvent type);
static void OnLivoxDeviceBroadcast(const BroadcastDeviceInfo *info);

_Livox::_Livox()
{
	m_nLivoxLidar = 0;
	m_bOpen = false;
}

_Livox::~_Livox()
{
}

bool _Livox::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;


//	string iName = "";
//	F_INFO(pK->v("_VisionBase", &iName));
//	m_pVB = (_VisionBase*) (pK->parent()->getChildInst(iName));

	return true;
}

bool _Livox::open(void)
{
	IF_Fl(!Init(),"Livox init failed");

	LivoxSdkVersion _sdkversion;
	GetLivoxSdkVersion(&_sdkversion);
	printf("Livox SDK version %d.%d.%d .\n", _sdkversion.major, _sdkversion.minor, _sdkversion.patch);

	memset(m_pLivoxDevice, 0, sizeof(m_pLivoxDevice));
	memset(m_pLivoxDataRecveiveCount, 0, sizeof(m_pLivoxDataRecveiveCount));

	SetBroadcastCallback(OnLivoxDeviceBroadcast);
	SetDeviceStateUpdateCallback(OnLivoxDeviceChange);

	if (!Start())
	{
		close();
		return false;
	}

	printf("Livox: Start\n");

	return true;
}

void _Livox::close(void)
{
	for (int i = 0; i < kMaxLidarCount; i++)
	{
		IF_CONT(m_pLivoxDevice[i].device_state != kDeviceStateSampling);

		LidarStopSampling(m_pLivoxDevice[i].handle, OnLivoxStopCallback, NULL);
	}

	Uninit();
}

bool _Livox::start(void)
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

void _Livox::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
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

bool _Livox::updateLidar(void)
{
	return true;
}

void LivoxGetLidarData(uint8_t handle, LivoxEthPacket *data, uint32_t data_num, void *client_data)
{
	NULL_(data);

	m_pLivoxDataRecveiveCount[handle] += data_num;
	if (m_pLivoxDataRecveiveCount[handle] % 10000 == 0)
	{
		printf("receive packet count %d %d\n", handle,
				m_pLivoxDataRecveiveCount[handle]);

		/** Parsing the timestamp and the point cloud data. */
		uint64_t cur_timestamp = *((uint64_t *) (data->timestamp));
		LivoxRawPoint *p_point_data = (LivoxRawPoint *) data->data;
	}
}

void OnLivoxCallback(uint8_t status, uint8_t handle, uint8_t response, void *data)
{
	printf("OnSampleCallback statue %d handle %d response %d \n", status,
			handle, response);
	if (status == kStatusSuccess)
	{
		if (response != 0)
		{
			m_pLivoxDevice[handle].device_state = kDeviceStateConnect;
		}
	}
	else if (status == kStatusTimeout)
	{
		m_pLivoxDevice[handle].device_state = kDeviceStateConnect;
	}
}

void OnLivoxStopCallback(uint8_t status, uint8_t handle, uint8_t response,void *data)
{
}

void OnLivoxDeviceInformation(uint8_t status, uint8_t handle, DeviceInformationResponse *ack, void *data)
{
	if (status != kStatusSuccess)
	{
		printf("Device Query Informations Failed %d\n", status);
	}
	if (ack)
	{
		printf("firm ver: %d.%d.%d.%d\n", ack->firmware_version[0],
				ack->firmware_version[1], ack->firmware_version[2],
				ack->firmware_version[3]);
	}
}

void OnLivoxDeviceChange(const DeviceInfo *info, DeviceEvent type)
{
	NULL_(info);

	printf("OnDeviceChange broadcast code %s update type %d\n",
			info->broadcast_code, type);
	uint8_t handle = info->handle;
	IF_(handle >= kMaxLidarCount);

	if (type == kEventConnect)
	{
		QueryDeviceInformation(handle, OnLivoxDeviceInformation, NULL);
		if (m_pLivoxDevice[handle].device_state == kDeviceStateDisconnect)
		{
			m_pLivoxDevice[handle].device_state = kDeviceStateConnect;
			m_pLivoxDevice[handle].info = *info;
		}
	}
	else if (type == kEventDisconnect)
	{
		m_pLivoxDevice[handle].device_state = kDeviceStateDisconnect;
	}
	else if (type == kEventStateChange)
	{
		m_pLivoxDevice[handle].info = *info;
	}

	if (m_pLivoxDevice[handle].device_state == kDeviceStateConnect)
	{
		printf("Device State error_code %d\n",
				m_pLivoxDevice[handle].info.status.status_code);
		printf("Device State working state %d\n", m_pLivoxDevice[handle].info.state);
		printf("Device feature %d\n", m_pLivoxDevice[handle].info.feature);
		if (m_pLivoxDevice[handle].info.state == kLidarStateNormal)
		{
			if (m_pLivoxDevice[handle].info.type == kDeviceTypeHub)
			{
				HubStartSampling(OnLivoxCallback, NULL);
			}
			else
			{
				LidarStartSampling(handle, OnLivoxCallback, NULL);
			}
			m_pLivoxDevice[handle].device_state = kDeviceStateSampling;
		}
	}
}

/** Callback function when broadcast message received.
 * You need to add listening device broadcast code and set the point cloud data callback in this function. */
void OnLivoxDeviceBroadcast(const BroadcastDeviceInfo *info)
{
	NULL_(info);
	printf("Receive Broadcast Code %s\n", info->broadcast_code);

	if (m_nLivoxLidar > 0)
	{
		bool found = false;
		int i = 0;
		for (i = 0; i < m_nLivoxLidar; ++i)
		{
			if (strncmp(info->broadcast_code, m_pLivoxBroadcastCodeList[i],
					kBroadcastCodeSize) == 0)
			{
				found = true;
				break;
			}
		}

		IF_(!found);
	}

	bool result = false;
	uint8_t handle = 0;
	result = AddLidarToConnect(info->broadcast_code, &handle);
	if (result == kStatusSuccess)
	{
		/** Set the point cloud data for a specific Livox LiDAR. */
		SetDataCallback(handle, LivoxGetLidarData, NULL);
		m_pLivoxDevice[handle].handle = handle;
		m_pLivoxDevice[handle].device_state = kDeviceStateDisconnect;
	}
}

void _Livox::draw(void)
{
	this->_ThreadBase::draw();
	string msg;

}

}
#endif

