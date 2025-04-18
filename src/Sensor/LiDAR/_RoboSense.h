/*
 * _RoboSense.h
 *
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor_LiDAR__RoboSense_H_
#define OpenKAI_src_Sensor_LiDAR__RoboSense_H_

#include "../../IO/_UDP.h"
#include "../../3D/PointCloud/_PCstream.h"
#include "../../Dependencies/SensorFusion/SensorFusion.h"
#include "../../Dependencies/CRC.h"
#include "../../Utility/util.h"
#include "../../Utility/utilNet.h"

namespace kai
{
	struct ROBOSENSE_DATA
	{
		uint8_t version;
		uint16_t length;
		uint16_t time_interval; /**< unit: 0.1 us */
		uint16_t dot_num;
		uint16_t udp_cnt;
		uint8_t frame_cnt;
		uint8_t data_type;
		uint8_t time_type;
		uint8_t rsvd[12];
		uint32_t crc32;
		uint8_t timestamp[8];
		uint8_t data[LVX2_N_DATA];
	};

	struct ROBOSENSE_CONFIG
	{
		uint8_t m_pclDataType;
		uint8_t m_patternMode;
		uint64_t m_hostIP;
		uint16_t m_hostPortState;
		uint16_t m_hostPortPCL;
		uint16_t m_hostPortIMU;
		uint8_t m_frameRate;
		uint8_t m_detectMode;
		uint8_t m_workModeAfterBoot;
		uint8_t m_workMode;
		uint8_t m_imuDataEn;

		vFloat2 m_vRz = {0.0, 500.0}; // z region

		void init(void)
		{
			m_pclDataType = kLivoxLidarCartesianCoordinateHighData;
			m_patternMode = kLivoxLidarScanPatternNoneRepetive;
			m_hostIP = 0;
			m_hostPortState = 0;
			m_hostPortPCL = 0;
			m_hostPortIMU = 0;
			m_frameRate = kLivoxLidarFrameRate25Hz;
			m_detectMode = kLivoxLidarDetectNormal;
			m_workModeAfterBoot = kLivoxLidarWorkModeAfterBootNormal;
			m_workMode = kLivoxLidarNormal;
			m_imuDataEn = 1;

			m_vRz = {0.0, 500.0};
		}
	};

	class _RoboSense : public _PCstream
	{
	public:
		_RoboSense();
		~_RoboSense();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual int start(void);
		virtual void console(void *pConsole);

		LVX2_CONFIG getConfig(void);
		void setConfig(const LVX2_CONFIG& cfg);

	private:
		// Common
		bool recvLivoxCmd(_IObase *pIO, LIVOX2_CMD *pResvCmd, bool bParity = false);
		bool recvLivoxData(_IObase *pIO, LIVOX2_DATA *pResvDATA, bool bParity = false);

		// Device Type Query
		void sendDeviceQuery(void);
		void updateWdeviceQuery(void);
		static void *getUpdateWdeviceQuery(void *This)
		{
			((_RoboSense *)This)->updateWdeviceQuery();
			return NULL;
		}

		void handleDeviceQuery(const LIVOX2_CMD &cmd);
		void updateRdeviceQuery(void);
		static void *getUpdateRdeviceQuery(void *This)
		{
			((_RoboSense *)This)->updateRdeviceQuery();
			return NULL;
		}

		// Control Command
		void setLvxPCLdataType(void);
		void setLvxPattern(void);
		void setLvxHost(void);
		void setLvxFrameRate(void);
		void setLvxDetectMode(void);
		void setLvxWorkModeAfterBoot(void);
		void setLvxWorkMode(void);
		void setLvxIMUdataEn(void);

		void getLvxConfig(void);
		void updateWctrlCmd(void);
		static void *getUpdateWctrlCmd(void *This)
		{
			((_RoboSense *)This)->updateWctrlCmd();
			return NULL;
		}

		void handleCtrlCmdAck(const LIVOX2_CMD &cmd);
		void updateRctrlCmd(void);
		static void *getUpdateRctrlCmd(void *This)
		{
			((_RoboSense *)This)->updateRctrlCmd();
			return NULL;
		}

		// Push command
		void handlePushCmd(const LIVOX2_CMD &cmd);
		void updateRpushCmd(void);
		static void *getUpdateRpushCmd(void *This)
		{
			((_RoboSense *)This)->updateRpushCmd();
			return NULL;
		}

		// Point Cloud Data
		void handlePointCloudData(const LIVOX2_DATA &d);
		void updateRpointCloud(void);
		static void *getUpdateRpointCloud(void *This)
		{
			((_RoboSense *)This)->updateRpointCloud();
			return NULL;
		}

		// IMU
		void handleIMUdata(const LIVOX2_DATA &d);
		void updateRimu(void);
		static void *getUpdateRimu(void *This)
		{
			((_RoboSense *)This)->updateRimu();
			return NULL;
		}

	protected:
		_Thread *m_pTdeviceQueryR;
		_Thread *m_pTctrlCmdW;
		_Thread *m_pTctrlCmdR;
		_Thread *m_pTpushCmdR;
		_Thread *m_pTpclR;
		_Thread *m_pTimuR;

		_UDP *m_pUDPdeviceQuery;
		_UDP *m_pUDPctrlCmd;
		_UDP *m_pUDPpushCmd;
		_UDP *m_pUDPpcl;
		_UDP *m_pUDPimu;
		_UDP *m_pUDPlog;

		// lvx state
		LVX2_STATE m_lvxState;
		TIME_OUT m_lvxTout;

		// lvx info
		string m_lvxSN;
		uint8_t m_pLvxSN[16];
		uint32_t m_lvxIP;
		uint16_t m_lvxCmdPort;
		uint8_t m_lvxDevType;

		// lvx config
		LVX2_CONFIG m_lvxCfg;

		// lvx IMU
		SF m_SF;
		uint64_t m_tIMU;
	};

}
#endif
