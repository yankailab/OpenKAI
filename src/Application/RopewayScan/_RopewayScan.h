/*
 * _RopewayScan.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_RopewayScan__RopewayScan_H_
#define OpenKAI_src_Application_RopewayScan__RopewayScan_H_

#include "../../Protocol/_JSONbase.h"
#include "../../Navigation/_NavBase.h"
#include "../../Actuator/_ActuatorBase.h"
#include "../../Utility/BitFlag.h"
//#include "_RopewayScanLivox.h"
//#include "_RopewayScanVz.h"

namespace kai
{
    enum ROPEWAYSCAN_THREAD_BIT
	{
		rws_reset,
		rws_move,
		rws_stop,
		rws_take,
		rws_save,
	};

	class _RopewayScan : public _JSONbase
	{
	public:
		_RopewayScan();
		virtual ~_RopewayScan();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	private:
		// main thread
		void reset(void);
		void move(void);
		void hold(void);
		void take(void);
		void save(void);
		void scanUpdate(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_RopewayScan *)This)->update();
			return NULL;
		}

		// UI handler
		void reset(picojson::object &o);
		void move(picojson::object &o);
		void stop(picojson::object &o);
		void take(picojson::object &o);
		void save(picojson::object &o);
		void handleMsg(string &str);
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_RopewayScan *)This)->updateR();
			return NULL;
		}

		// UI notifier
		void send(void);
		void sendHeartbeat(void);
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_RopewayScan *)This)->updateW();
			return NULL;
		}

	protected:
//		_NavBase *m_pNav;
		_Thread *m_pTw;
		_ActuatorBase* m_pAct;
		uint8_t m_iAct;
//		_RopewayScanLivox* m_pLivox;
//		_RopewayScanVz* m_pVz;

		float m_p;			// pos along the axis
		float m_v;			// vector = spd * direction
		float m_dist;		// interval between scanning positions
		float m_spd;		// unit speed
		float m_direction;

		string m_msg;

		BIT_FLAG m_fProcess;
		INTERVAL_EVENT m_ieSendHB;
	};

}
#endif
