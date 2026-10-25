/*
 * _Livox2.h
 *
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor_LiDAR_Livox__Livox2_H_
#define OpenKAI_src_Sensor_LiDAR_Livox__Livox2_H_

#include "../../../Universe/_TransformRigid.h"
#include "../../../DataObject/PCLframe.h"
#include "../../../DataObject/IMUstream.h"
#include "../../../Dependencies/SensorFusion/SensorFusion.h"
#include "../../../Dependencies/CRC.h"
#include "../../../Utility/util.h"
#include "../../../Utility/utilNet.h"

#include "livox_lidar_def.h"
#include "Livox2Protocol.h"
#include "../../IMU/IMUpreview.h"
#include <array>
#include <map>
#include <mutex>

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
            memset(this, 0, sizeof(*this));
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

		void addData(const void *pD, int nD)
		{
            if (nD < 0 || m_iD + nD > LVX2_N_DATA) throw std::length_error("Livox command payload too large");
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
			addData(&v, 1);
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
		uint32_t m_hostIP;
		uint16_t m_hostPortState;
		uint16_t m_hostPortPCL;
		uint16_t m_hostPortIMU;
		uint8_t m_frameRate;
		uint8_t m_detectMode;
		uint8_t m_workModeAfterBoot;
		uint8_t m_workMode;
		uint8_t m_imuDataEn;
		Vector2f m_vRz = Vector2f::Zero(); // z region

		void init(void)
		{
			m_pclDataType = kLivoxLidarCartesianCoordinateHighData;
			m_patternMode = kLivoxLidarScanPatternNoneRepetive;
			m_hostIP = 0;
			m_hostPortState = 56201;
			m_hostPortPCL = 56301;
			m_hostPortIMU = 56401;
			m_frameRate = kLivoxLidarFrameRate25Hz;
			m_detectMode = kLivoxLidarDetectNormal;
			m_workModeAfterBoot = kLivoxLidarWorkModeAfterBootNormal;
			m_workMode = kLivoxLidarNormal;
			m_imuDataEn = 1;

			m_vRz = Vector2f(0.0, 500.0);
		}
	};

	class _Livox2 : public _ModuleBase
	{
        friend struct Livox2TestAccess;
	public:
		_Livox2();
		~_Livox2();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool check(void);
		virtual bool start(void);
        void stop(void) override;
		virtual void console(void *pConsole);
        void console(const json &j, void *pJSONbase) override;
		virtual void clear(void);

		LVX2_CONFIG getConfig(void);
		void setConfig(const LVX2_CONFIG &cfg);

    private:
        enum SocketChannel { Query, Command, Push, PointCloud, Imu, SocketCount };
        bool openSockets(string &error);
        void closeSockets();
        bool sendDatagram(SocketChannel channel, uint16_t port, const LIVOX2_CMD &cmd);
        void update();
        static void *getUpdate(void *self) { static_cast<_Livox2 *>(self)->update(); return nullptr; }
        void receive(size_t channel);
        void service(uint64_t now);
        bool recvLivoxCmd(const uint8_t *bytes, size_t length, LIVOX2_CMD *cmd, bool parity = true);
        bool recvLivoxData(const uint8_t *bytes, size_t length, LIVOX2_DATA *data, bool parity = true);
        void sendDeviceQuery();
        void handleDeviceQuery(const LIVOX2_CMD &cmd);
        void sendCommand(LIVOX2_CMD cmd);
        void setLvxByte(uint16_t key, uint8_t value);
        void setLvxHost();
        void configureDevice();
        void getLvxConfig();
        void handleCtrlCmdAck(const LIVOX2_CMD &cmd);
        void handleInfo(const uint8_t *data, size_t length, uint16_t count);
        void handlePushCmd(const LIVOX2_CMD &cmd);
        void handlePointCloudData(const LIVOX2_DATA &data);
        void handleIMUdata(const LIVOX2_DATA &data);
        json configValues() const;
        json controlSchema() const;
        json imuValues() const;
        bool applyConfig(const json &patch, bool live, json &errors);
        void resetFrame();

    protected:
        _TransformRigid *m_pTf = nullptr;
        PCLframe *m_pPCLout = nullptr;
        std::array<int, SocketCount> m_sockets{{-1, -1, -1, -1, -1}};
        bool m_started = false;
        uint32_t m_hostIP = 0; // Actual local address, resolved from the route when lvxHostIP is zero.
        uint16_t m_queryPort = 56000;
        uint16_t m_hostPortCmd = 56101;
        int m_receiveBufferBytes = 4194304;
        uint64_t m_lastDiscovery = 0;

        // All driver state is protected by m_mutex, including console snapshots.
        mutable std::recursive_mutex m_mutex;
        LVX2_STATE m_lvxState = lvxState_deviceQuery;
        uint64_t m_timeoutNs = 10 * NSEC_SEC;
        uint64_t m_lastRx = 0;
        uint64_t m_lastQuery = 0;
        uint32_t m_sequence = 0;
        struct PendingCommand
        {
            LIVOX2_CMD command;
            uint64_t sent = 0;
            unsigned attempts = 1;
        };
        std::map<uint32_t, PendingCommand> m_pending;
        bool m_configFailed = false;
        string m_lastError;
        json m_observed = json::object();

        string m_lvxSN;
        string m_detectedSN;
        uint32_t m_lvxIP = 0;
        uint16_t m_lvxCmdPort = 0;
        uint8_t m_lvxDevType = 0;
        LVX2_CONFIG m_lvxCfg;

        Quaterniond m_mountOrientation = Quaterniond::Identity();
        bool m_bIMUpreview = false;
        bool m_bIMUstab = false;
        IMUpreview m_imuPreview;
        uint64_t m_tIMU = 0;
        uint64_t m_lastIMUrx = 0;
        IMUstream *m_pIMUout = nullptr;

        vector<GEOMETRY_POINT> m_vFramePoints;
        Vector3f m_vColorDefault = Vector3f::Ones();
        Vector2f m_vRangeD = Vector2f(0.1f, 200.0f);
        bool m_bReflectivity = true;
        int m_nMaxFramePoints = 100000;
        uint64_t m_frameDurationNs = 100 * NSEC_MSEC;
        uint64_t m_tPCLframe = 0;
        uint64_t m_tLastPCL = 0;
        uint8_t m_iPCLframe = 0;
        bool m_bPCLframe = false;
        uint64_t m_pointPackets = 0;
        uint64_t m_points = 0;
        uint64_t m_frames = 0;
        uint64_t m_imuSamples = 0;
        uint64_t m_invalidPackets = 0;
        uint64_t m_outOfOrderPackets = 0;
        uint64_t m_frameOverflows = 0;
        uint64_t m_commandErrors = 0;
        uint64_t m_commandRetries = 0;
    };

}
#endif
