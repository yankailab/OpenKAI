/*
 * _Livox2.h
 *
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor_LiDAR__Livox2_H_
#define OpenKAI_src_Sensor_LiDAR__Livox2_H_

#include "../../IO/_UDP.h"
#include "../../3D/PointCloud/_PCstream.h"
#include "../../Dependencies/SensorFusion/SensorFusion.h"
#include "../../Dependencies/CRC.h"
#include "../../Utility/util.h"
#include "../../Utility/utilNet.h"

#include "livox_lidar_def.h"

#define LVX2_N_BUF 1380
#define LVX2_N_DATA 1344
#define LVX2_N_SN 16
#define LVX2_CMD_N_HDR 24
#define LVX2_DATA_N_HDR 28
#define LVX2_SOF 0xAA

#define LVX2_CMD_REQ 0x00
#define LVX2_CMD_ACK 0x01

#define LVX2_CMD_DISCOVER 0x0000
#define LVX2_CMD_SET 0x0100
#define LVX2_CMD_GET 0x0101
#define LVX2_CMD_PUSH 0x0102

#define LVX2_CMD_REBOOT 0x0200
#define LVX2_CMD_RST_FACTORY 0x0201
#define LVX2_CMD_SET_GOS_TSTAMP 0x0202
#define LVX2_CMD_GET_LOG 0x0300

#define LVX2_RET_SUCCESS 0x00

namespace kai
{
	struct LIVOX2_DATA
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

	struct LIVOX2_CMD
	{
		uint8_t sof;
		uint8_t version;
		uint16_t length;
		uint32_t seq_num;
		uint16_t cmd_id;
		uint8_t cmd_type;
		uint8_t sender_type;
		char rsvd[6];
		uint16_t crc16_h;
		uint32_t crc32_d;
		uint8_t data[LVX2_N_DATA];
		int m_iD;

		void init(uint16_t cmdID, uint8_t cmdType, uint32_t seqNum)
		{
			sof = LVX2_SOF;
			version = 0;
			length = LVX2_CMD_N_HDR;
			seq_num = seqNum;
			cmd_id = cmdID;
			cmd_type = cmdType; // REQ
			sender_type = 0x00; // Host computer
			m_iD = 0;
		}

		void calcCRC(void)
		{
			crc16_h = CRC::Calculate(this, 18, CRC::CRC_16_CCITTFALSE());
			crc32_d = 0;
			if (length > LVX2_CMD_N_HDR)
				crc32_d = CRC::Calculate(data, length - LVX2_CMD_N_HDR, CRC::CRC_32());
		}

		void addData(void *pD, int nD)
		{
			memcpy(&data[m_iD], pD, nD);
			m_iD += nD;
			length += nD;
		}

		void addData(uint16_t v)
		{
			addData(&v, 2);
		}

		void addData(uint8_t v)
		{
			data[m_iD++] = v;
			length++;
		}
	};

	enum LVX2_STATE
	{
		lvxState_deviceQuery = 0,
		lvxState_init = 1,
		lvxState_work = 2,
	};

	struct LVX2_CONFIG
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
		vFloat2 m_vRz; // z region

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

			m_vRz.set(0.0, 500.0);
		}
	};

	class _Livox2 : public _PCstream
	{
	public:
		_Livox2();
		~_Livox2();

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
			((_Livox2 *)This)->updateWdeviceQuery();
			return NULL;
		}

		void handleDeviceQuery(const LIVOX2_CMD &cmd);
		void updateRdeviceQuery(void);
		static void *getUpdateRdeviceQuery(void *This)
		{
			((_Livox2 *)This)->updateRdeviceQuery();
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
			((_Livox2 *)This)->updateWctrlCmd();
			return NULL;
		}

		void handleCtrlCmdAck(const LIVOX2_CMD &cmd);
		void updateRctrlCmd(void);
		static void *getUpdateRctrlCmd(void *This)
		{
			((_Livox2 *)This)->updateRctrlCmd();
			return NULL;
		}

		// Push command
		void handlePushCmd(const LIVOX2_CMD &cmd);
		void updateRpushCmd(void);
		static void *getUpdateRpushCmd(void *This)
		{
			((_Livox2 *)This)->updateRpushCmd();
			return NULL;
		}

		// Point Cloud Data
		void handlePointCloudData(const LIVOX2_DATA &d);
		void updateRpointCloud(void);
		static void *getUpdateRpointCloud(void *This)
		{
			((_Livox2 *)This)->updateRpointCloud();
			return NULL;
		}

		// IMU
		void handleIMUdata(const LIVOX2_DATA &d);
		void updateRimu(void);
		static void *getUpdateRimu(void *This)
		{
			((_Livox2 *)This)->updateRimu();
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
