/*
 * _RoboSenseAiry.h
 *
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor_LiDAR__RoboSenseAiry_H_
#define OpenKAI_src_Sensor_LiDAR__RoboSenseAiry_H_

#include "../../IO/_UDP.h"
#include "../../3D/PointCloud/_PCstream.h"
#include "../../Dependencies/SensorFusion/SensorFusion.h"
#include "../../Dependencies/CRC.h"
#include "../../Utility/util.h"
#include "../../Utility/utilNet.h"

#define RS_MSOP_N 1248
#define RS_DIFOP_N 1248

namespace kai
{
	struct RS_MSOP_HDR
	{
		uint32_t m_pktHead;
		uint32_t m_pREV0[2];
		uint32_t m_pktCnt;
		uint32_t m_REV;
		uint8_t m_pTimestamp[10];
		uint8_t m_REV2;
		uint8_t m_lidarType;
		uint8_t m_lidarModel;
		uint8_t m_pRev[9];
	};

	struct RS_MSOP_DB
	{
		uint16_t m_flasg;	//0xffee
		uint16_t m_azimuth;
		uint8_t m_pD[144];
	};

	struct RS_MSOP
	{
		RS_MSOP_HDR m_hdr;
		RS_MSOP_DB mpDblock[8];
		uint8_t m_pEndFlag[6];
		uint8_t m_pResv[16];
	};

	struct RS_DIFOP
	{
		uint8_t m_pHdr[8];		// 0xA5, 0xFF, 0x00, 0x5A, 0x11, 0x11, 0x55, 0x55
		uint16_t m_motSpdSet;
		uint32_t m_IPsrc;
		uint32_t m_IPdst;
		uint8_t m_pMAC[6];
		uint16_t m_MSOPport;
		uint16_t m_resv;
		uint16_t m_DIFOPport;
		uint16_t m_resv2;
		uint8_t m_pVerFmMB[5];
		uint8_t m_pVerFmBB[5];
		uint8_t m_pVerFmAPP[5];
		uint8_t m_pVerFmMot[5];
		uint8_t m_pResv3[227];
		uint8_t m_pProdSN[6];
		uint8_t m_pResv4[3];
		uint8_t m_tSync;
		uint8_t m_tSyncState;
		uint8_t m_pTime[10];
		uint8_t m_pResv5[60];
		uint16_t m_motSpdReal;
		uint8_t m_pResv6[93];
		uint8_t m_pCalibVangle[288];
		uint8_t m_pCalibHangle[288];
		uint16_t m_vInputMB;
		uint16_t m_vInput;
		uint16_t m_v12v;
		uint8_t m_pResv7[10];
		uint16_t m_mbEmitTemp;
		uint8_t m_pResv8[10];
		uint8_t m_pIMUcalib[28];
		uint8_t m_pResv9[126];
		uint8_t m_pFend[2];		// 0x0F 0xF0
	};

	class _RoboSenseAiry : public _PCstream
	{
	public:
		_RoboSenseAiry();
		~_RoboSenseAiry();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool check(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

	private:
		// MSOP
		bool recvMSOP(void);
		void updateMSOP(void);
		static void *getUpdateMSOP(void *This)
		{
			((_RoboSenseAiry *)This)->updateMSOP();
			return NULL;
		}

		// DIFOP
		bool recvDIFOP(void);
		void updateDIFOP(void);
		static void *getUpdateDIFOP(void *This)
		{
			((_RoboSenseAiry *)This)->updateDIFOP();
			return NULL;
		}

	protected:
		_Thread *m_pTdifop;

		_UDP *m_pUDPmsop;
		_UDP *m_pUDPdifop;

		// IMU
		SF m_SF;
		uint64_t m_tIMU;
	};

}
#endif
