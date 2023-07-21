/*
 * _SwarmBase.h
 *
 *  Created on: April 3, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Swarm_SwarmBase_H_
#define OpenKAI_src_Swarm_SwarmBase_H_

#include "../Base/_ModuleBase.h"

namespace kai
{
	enum SWMSG_TYPE
	{
		swMsg_hB = 0,
		swMsg_cmd_setState = 1,
		swMsg_gc_update = 2,
	};

	enum SWMSG_CMD_STATE
	{
		swMsg_state_standBy = 0,
		swMsg_state_takeOff = 1,
		swMsg_state_auto = 2,
		swMsg_state_RTL = 3,
		swMsg_state_land = 4,
	};

	struct SWMSG_HB
	{
		const static uint8_t m_nB = 34;

		const static uint8_t m_mType = swMsg_hB; // 0
		uint64_t m_srcID;	// 1
		uint64_t m_lat; // 9
		uint64_t m_lng; // 17
		uint16_t m_alt; // 25
		uint16_t m_hdg; // 27
		uint16_t m_spd; // 29
		uint8_t m_batt; // 31
		uint8_t m_iMsg; // 32
		uint8_t m_checksum; //33

		bool decode(const uint8_t* pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);
			IF_F(pB[0] != m_mType);

			m_srcID = *((uint64_t*)&pB[1]);
			m_lat = *((uint64_t*)&pB[9]);
			m_lng = *((uint64_t*)&pB[17]);
			m_alt = *((uint16_t*)&pB[25]);
			m_hdg = *((uint16_t*)&pB[27]);
			m_spd = *((uint16_t*)&pB[29]);
			m_batt = *((uint8_t*)&pB[31]);
			m_iMsg = *((uint8_t*)&pB[32]);
			m_checksum = *((uint8_t*)&pB[33]);

			return true;
		}

		int encode(uint8_t* pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);

			*((uint8_t*)&pB[0]) = m_mType;
			*((uint64_t*)&pB[1]) = m_srcID;
			*((uint64_t*)&pB[9]) = m_lat;
			*((uint64_t*)&pB[17]) = m_lng;
			*((uint16_t*)&pB[25]) = m_alt;
			*((uint16_t*)&pB[27]) = m_hdg;
			*((uint16_t*)&pB[29]) = m_spd;
			*((uint8_t*)&pB[31]) = m_batt;
			*((uint8_t*)&pB[32]) = m_iMsg;
			*((uint8_t*)&pB[33]) = m_checksum;
		}
	};

	struct SWMSG_CMD_SETSTATE
	{
		const static uint8_t m_nB = 20;

		const static uint8_t m_mType = swMsg_cmd_setState; // 0
		uint64_t m_srcID;	// 1
		uint64_t m_dstID;	// 9
		uint8_t m_state;	// 17
		uint8_t m_iMsg;		// 18
		uint8_t m_checksum; // 19

		bool decode(const uint8_t* pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);
			IF_F(pB[0] != m_mType);

			m_srcID = *((uint64_t*)&pB[1]);
			m_dstID = *((uint64_t*)&pB[9]);
			m_state = *((uint64_t*)&pB[17]);
			m_iMsg = *((uint8_t*)&pB[18]);
			m_checksum = *((uint8_t*)&pB[19]);

			return true;
		}

		int encode(uint8_t* pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);

			*((uint8_t*)&pB[0]) = m_mType;
			*((uint64_t*)&pB[1]) = m_srcID;
			*((uint64_t*)&pB[9]) = m_dstID;
			*((uint64_t*)&pB[17]) = m_state;
			*((uint16_t*)&pB[18]) = m_iMsg;
			*((uint8_t*)&pB[19]) = m_checksum;
		}
	};

	struct SWMSG_GC_UPDATE
	{
		const static uint8_t m_nB = 29;

		const static uint8_t m_mType = swMsg_gc_update; // 0
		uint64_t m_srcID;	// 1
		uint64_t m_dstID;	// 9
		uint64_t m_iGC;		// 17
		uint16_t m_w;		// 25
		uint8_t m_iMsg;		// 27
		uint8_t m_checksum; // 28

		bool decode(const uint8_t* pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);
			IF_F(pB[0] != m_mType);

			m_srcID = *((uint64_t*)&pB[1]);
			m_dstID = *((uint64_t*)&pB[9]);
			m_iGC = *((uint64_t*)&pB[17]);
			m_w = *((uint16_t*)&pB[25]);
			m_iMsg = *((uint8_t*)&pB[27]);
			m_checksum = *((uint8_t*)&pB[28]);

			return true;
		}

		int encode(uint8_t* pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);

			*((uint8_t*)&pB[0]) = m_mType;
			*((uint64_t*)&pB[1]) = m_srcID;
			*((uint64_t*)&pB[9]) = m_dstID;
			*((uint64_t*)&pB[17]) = m_iGC;
			*((uint64_t*)&pB[25]) = m_w;
			*((uint16_t*)&pB[27]) = m_iMsg;
			*((uint8_t*)&pB[28]) = m_checksum;
		}
	};

	struct SWARM_NODE
	{
		uint64_t m_id;
		vDouble2 m_vPos = {0,0};
		float	m_alt = 0;
		float	m_spd = 0;
		float	m_batt = 0;
		uint8_t	m_iMsg = 0;
		uint64_t m_tLastUpdate = 0;
	};

	class _SwarmBase : public _ModuleBase
	{
	public:
		_SwarmBase();
		virtual ~_SwarmBase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		// swarm msg handlers
		void handleMsgHB(const SWMSG_HB& m);

	protected:
		int getNodeIdx(const uint64_t& ID);
		SWARM_NODE* getNode(const uint64_t& ID);
		bool addNode(const SWARM_NODE& n);
		bool updateNode(const SWARM_NODE& n);
		void deleteNode(const uint64_t& ID);
		void deleteNode(int i);

		void deleteExpiredNodes(const uint64_t& tExpire);
		SWARM_NODE* findClosestNode(vDouble2 vPos);
		int getNodesWithinRadius(const vDouble2& vPos, float r);

	protected:
		vector<SWARM_NODE> m_vNodes;

		uint64_t m_tExpire;
		
	};

}
#endif
