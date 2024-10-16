/*
 * _Livox2.h
 *
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor_LiDAR__Livox2_H_
#define OpenKAI_src_Sensor_LiDAR__Livox2_H_

#include "../../../IO/_IObase.h"
#include "../../../3D/PointCloud/_PCstream.h"
#include "../../../Dependencies/SensorFusion/SensorFusion.h"
#include "../../../Dependencies/CRC.h"

#include "livox_lidar_def.h"
#include "livox_lidar_api.h"

#define LIVOX2_N_DATA 1400
#define LIVOX2_SOF 0xAA

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
		uint8_t data[LIVOX2_N_DATA];
	};

	struct LIVOX2_DATA_RECV
	{
		LIVOX2_DATA m_cmd;
		int m_iB;

		void reset(void)
		{
			m_iB = 0;
		}
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
		uint8_t data[LIVOX2_N_DATA];
	};

	struct LIVOX2_CMD_RECV
	{
		LIVOX2_CMD m_cmd;
		int m_iB;
		int m_nB;

		void reset(void)
		{
			m_iB = 0;
			m_nB = 24;
		}
	};

	enum LIVOX2_STATE
	{
		livox2_deviceQuery = 0,
		livox2_work = 1,
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
		// Device Type Query
		void sendDeviceQuery(void);
		void updateWdeviceQuery(void);
		static void *getUpdateWdeviceQuery(void *This)
		{
			((_Livox2 *)This)->updateWdeviceQuery();
			return NULL;
		}

		void handleDeviceQuery(const LIVOX2_CMD& cmd);
		bool recvDeviceQuery(void);
		void updateRdeviceQuery(void);
		static void *getUpdateRdeviceQuery(void *This)
		{
			((_Livox2 *)This)->updateRdeviceQuery();
			return NULL;
		}

		// Control Command
		void updateCtrlCmd(void);
		void updateWctrlCmd(void);
		static void *getUpdateWctrlCmd(void *This)
		{
			((_Livox2 *)This)->updateWctrlCmd();
			return NULL;
		}

		void updateRctrlCmd(void);
		static void *getUpdateRctrlCmd(void *This)
		{
			((_Livox2 *)This)->updateRctrlCmd();
			return NULL;
		}

		// Push command
		void recvWorkMode(livox_status status, LivoxLidarAsyncControlResponse *pR);
		void recvLidarInfoChange(const LivoxLidarInfo *pI);
		void updateRpushCmd(void);
		static void *getUpdateRpushCmd(void *This)
		{
			((_Livox2 *)This)->updateRpushCmd();
			return NULL;
		}

		// Point Cloud Data
		void recvPointCloud(LivoxLidarEthernetPacket *pD);
		void updateRpointCloud(void);
		static void *getUpdateRpointCloud(void *This)
		{
			((_Livox2 *)This)->updateRpointCloud();
			return NULL;
		}

		// IMU
		void recvIMU(LivoxLidarEthernetPacket *pD);
		void updateRimu(void);
		static void *getUpdateRimu(void *This)
		{
			((_Livox2 *)This)->updateRimu();
			return NULL;
		}

	protected:
		_Thread *m_pTdeviceQueryR;
		_Thread *m_pTcontrolCmdW;
		_Thread *m_pTcontrolCmdR;
		_Thread *m_pTpushCmdR;
		_Thread *m_pTpointCloudR;
		_Thread *m_pTimuR;

		_IObase *m_pDeviceQuery;
		_IObase *m_pCtrlCmd;
		_IObase *m_pPushCmd;
		_IObase *m_pPointCloud;
		_IObase *m_pIMU;
		_IObase *m_pLog;

		LIVOX2_CMD_RECV m_recvDeviceQuery;

		LIVOX2_STATE m_state;
		string m_SN;
		LivoxLidarWorkMode m_workMode;

		SF m_SF;
		uint64_t m_tIMU;
		bool m_bEnableIMU;
	};

}
#endif
