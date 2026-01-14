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
		const static uint8_t m_nB = 29;
		uint64_t m_srcNetAddr = 0;

		const static uint8_t m_mType = swMsg_hB; // 0
		uint16_t m_srcID = 0;					 // 1
		int64_t m_lat = 0;						 // 3			1e7
		int64_t m_lng = 0;						 // 11		1e7
		int16_t m_alt = 0;						 // 19		1e2
		int16_t m_hdg = 0;						 // 21		1e1
		int16_t m_spd = 0;						 // 23		1e2
		uint8_t m_batt = 0;						 // 25		1~100
		uint8_t m_mode = 0;						 // 26
		uint8_t m_iMsg = 0;						 // 27
		uint8_t m_checksum = 0;					 // 28

		bool decode(const uint8_t *pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);
			IF_F(pB[0] != m_mType);

			m_srcID = *((uint16_t *)&pB[1]);
			m_lat = *((int64_t *)&pB[3]);
			m_lng = *((int64_t *)&pB[11]);
			m_alt = *((int16_t *)&pB[19]);
			m_hdg = *((int16_t *)&pB[21]);
			m_spd = *((int16_t *)&pB[23]);
			m_batt = *((uint8_t *)&pB[25]);
			m_mode = *((uint8_t *)&pB[26]);
			m_iMsg = *((uint8_t *)&pB[27]);
			m_checksum = *((uint8_t *)&pB[28]);

			return true;
		}

		int encode(uint8_t *pB, int nB)
		{
			IF__(nB < m_nB, -1);
			NULL__(pB, -1);

			*((uint8_t *)&pB[0]) = m_mType;
			*((uint16_t *)&pB[1]) = m_srcID;
			*((int64_t *)&pB[3]) = m_lat;
			*((int64_t *)&pB[11]) = m_lng;
			*((int16_t *)&pB[19]) = m_alt;
			*((int16_t *)&pB[21]) = m_hdg;
			*((int16_t *)&pB[23]) = m_spd;
			*((uint8_t *)&pB[25]) = m_batt;
			*((uint8_t *)&pB[26]) = m_mode;
			*((uint8_t *)&pB[27]) = m_iMsg;
			*((uint8_t *)&pB[28]) = m_checksum;

			return m_nB;
		}
	};

	struct SWMSG_CMD_SETSTATE
	{
		const static uint8_t m_nB = 8;
		uint64_t m_srcNetAddr = 0;

		const static uint8_t m_mType = swMsg_cmd_setState; // 0
		uint16_t m_srcID;								   // 1
		uint16_t m_dstID;								   // 3
		uint8_t m_state;								   // 5
		uint8_t m_iMsg;									   // 6
		uint8_t m_checksum;								   // 7

		bool decode(const uint8_t *pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);
			IF_F(pB[0] != m_mType);

			m_srcID = *((uint16_t *)&pB[1]);
			m_dstID = *((uint16_t *)&pB[3]);
			m_state = *((uint8_t *)&pB[5]);
			m_iMsg = *((uint8_t *)&pB[6]);
			m_checksum = *((uint8_t *)&pB[7]);

			return true;
		}

		int encode(uint8_t *pB, int nB)
		{
			IF__(nB < m_nB, -1);
			NULL__(pB, -1);

			*((uint8_t *)&pB[0]) = m_mType;
			*((uint16_t *)&pB[1]) = m_srcID;
			*((uint16_t *)&pB[3]) = m_dstID;
			*((uint8_t *)&pB[5]) = m_state;
			*((uint8_t *)&pB[6]) = m_iMsg;
			*((uint8_t *)&pB[7]) = m_checksum;

			return m_nB;
		}
	};

	struct SWMSG_GC_UPDATE
	{
		const static uint8_t m_nB = 13;
		uint64_t m_srcNetAddr = 0;

		const static uint8_t m_mType = swMsg_gc_update; // 0
		uint16_t m_srcID;								// 1
		uint16_t m_dstID;								// 3
		uint32_t m_iGC;									// 5
		uint16_t m_w;									// 9
		uint8_t m_iMsg;									// 11
		uint8_t m_checksum;								// 12

		bool decode(const uint8_t *pB, int nB)
		{
			IF_F(nB < m_nB);
			NULL_F(pB);
			IF_F(pB[0] != m_mType);

			m_srcID = *((uint16_t *)&pB[1]);
			m_dstID = *((uint16_t *)&pB[3]);
			m_iGC = *((uint32_t *)&pB[5]);
			m_w = *((uint16_t *)&pB[9]);
			m_iMsg = *((uint8_t *)&pB[11]);
			m_checksum = *((uint8_t *)&pB[12]);

			return true;
		}

		int encode(uint8_t *pB, int nB)
		{
			IF__(nB < m_nB, -1);
			NULL__(pB, -1);

			*((uint8_t *)&pB[0]) = m_mType;
			*((uint16_t *)&pB[1]) = m_srcID;
			*((uint16_t *)&pB[3]) = m_dstID;
			*((uint32_t *)&pB[5]) = m_iGC;
			*((uint16_t *)&pB[9]) = m_w;
			*((uint8_t *)&pB[11]) = m_iMsg;
			*((uint8_t *)&pB[12]) = m_checksum;

			return m_nB;
		}
	};

	struct SWARM_NODE
	{
		uint64_t m_srcNetAddr = 0;
		uint16_t m_id = 0;
		bool m_bPosValid = false;
		vDouble2 m_vPos = {0, 0};
		float m_alt = 0;
		float m_hdg = 0;
		float m_spd = 0;
		float m_batt = 0;
		uint8_t m_mode = 0;
		uint8_t m_iMsg = 0;
		uint64_t m_tLastUpdate = 0;
	};

	class _SwarmBase : public _ModuleBase
	{
	public:
		_SwarmBase();
		virtual ~_SwarmBase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool check(void);
		virtual void console(void *pConsole);

		// swarm msg handlers
		void handleMsgHB(const SWMSG_HB &m);
		SWARM_NODE *getNodeByID(uint16_t id);
		SWARM_NODE *getNodeByIDrange(vInt2 vID);
		SWARM_NODE *getNode(int i);

	protected:
		// node control
		int getNodeIdx(const uint64_t &ID);
		SWARM_NODE *getNode(const uint64_t &ID);
		SWARM_NODE *addNode(const SWARM_NODE &n);
		bool updateNode(const SWARM_NODE &n);
		void deleteNode(const uint64_t &ID);
		void deleteNode(int i);

		void deleteExpiredNodes(const uint64_t &tExpire);
		SWARM_NODE *findClosestNode(vDouble2 vPos);
		int getNodesWithinRadius(const vDouble2 &vPos, float r);

	protected:
		vector<SWARM_NODE> m_vNodes;

		uint64_t m_tExpire;
	};

}
#endif
