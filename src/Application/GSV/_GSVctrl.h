/*
 * _GSVctrl.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_GSV__GSVctrl_H_
#define OpenKAI_src_Application_GSV__GSVctrl_H_

#include "../../Protocol/_JSONbase.h"

namespace kai
{

	class _GSVctrl : public _JSONbase
	{
	public:
		_GSVctrl();
		virtual ~_GSVctrl();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	private:
		// main thread
		void send(void);
		void sendHeartbeat(void);
		void sendConfig(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_GSVctrl *)This)->update();
			return NULL;
		}

		// UI handler
		void reset(picojson::object &o);
		void start(picojson::object &o);
		void stop(picojson::object &o);
		void save(picojson::object &o);
		void setConfig(picojson::object &o);
		void getConfig(picojson::object &o);
		void handleMsg(string &str);
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_GSVctrl *)This)->updateR();
			return NULL;
		}


	protected:
//		_LivoxAutoScan* m_pLivox;
		string m_msg;
	};

}
#endif
