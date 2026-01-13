/*
 * _SwarmSearch.h
 *
 *  Created on: July 19, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_App_SwarmSearch_SwarmSearchSearch_H_
#define OpenKAI_src_App_SwarmSearch_SwarmSearchSearch_H_

#include "_SwarmBase.h"

namespace kai
{
    struct SWARM_SEARCH_STATE_NODE
    {
        int8_t m_iState;
        int8_t STANDBY;
        int8_t TAKEOFF;
        int8_t AUTO;
        int8_t RTL;

        bool bValid(void)
        {
            IF_F(STANDBY < 0);
            IF_F(TAKEOFF < 0);
            IF_F(AUTO < 0);
            IF_F(RTL < 0);

            return true;
        }

        void update(int8_t iState)
        {
            m_iState = iState;
        }

        bool bSTANDBY(void)
        {
            return (m_iState == STANDBY);
        }

        bool bTAKEOFF(void)
        {
            return (m_iState == TAKEOFF);
        }

        bool bAUTO(void)
        {
            return (m_iState == AUTO);
        }

        bool bRTL(void)
        {
            return (m_iState == RTL);
        }

		string getState(void)
		{
			IF__(m_iState == STANDBY, "standby");
			IF__(m_iState == TAKEOFF, "takeoff");
			IF__(m_iState == AUTO, "auto");
			IF__(m_iState == RTL, "rtl");

			return "unknown";
		}
    };

	struct GCELL_SEARCH
	{
		float m_w = 0;

		void clear(void)
		{
			m_w = 0;
		}
	};

	class _SwarmSearch : public _SwarmBase
	{
	public:
		_SwarmSearch();
		virtual ~_SwarmSearch();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
	    virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		// swarm msg handlers
		void handleMsgGCupdate(const SWMSG_GC_UPDATE& m);

		// grid cell
		void clearAllGridCells(void);

	protected:
		bool genGridCells(void);
		void optimizeRoute(void);
		void updateNodes(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_SwarmSearch *)This)->update();
			return NULL;
		}

	protected:
		GCELL_SEARCH* m_pGcell;
		int m_nGcell;

		SWARM_NODE m_myNode;
		INTERVAL_EVENT m_ieOptRoute;
	};

}
#endif
