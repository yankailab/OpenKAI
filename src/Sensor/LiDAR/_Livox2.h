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
		lvxState_config = 1,
		lvxState_work = 2,
	};

	struct Livox2Ctrl
	{
		vFloat2 m_vRz = {0.0, 500.0}; // z region
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

		void setLidarMode(LivoxLidarWorkMode m);

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
		void setLvxHost(void);
		void setLvxWorkMode(void);
		void setLvxPattern(void);
		void setLvxPCLdataType(void);
		void setLvxDetectMode(void);
		void updateCtrlCmd(void);
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
		void recvWorkMode(livox_status status, LivoxLidarAsyncControlResponse *pR);
		void recvLidarInfoChange(const LivoxLidarInfo *pI);
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

		// lvx config
		string m_lvxSN;
		uint8_t m_pLvxSN[16];
		uint64_t m_lvxIP;
		uint16_t m_lvxCmdPort;
		uint8_t m_lvxDevType;

		// lvx state
		LVX2_STATE m_lvxState;
		LivoxLidarWorkMode m_lvxWorkMode;

		// lvx IMU
		SF m_SF;
		uint64_t m_tIMU;
		bool m_bEnableIMU;

		// host config
		uint64_t m_hostIP;
		uint16_t m_hostPortState;
		uint16_t m_hostPortPCL;
		uint16_t m_hostPortIMU;

	};

}
#endif
