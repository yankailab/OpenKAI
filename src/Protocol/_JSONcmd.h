/*
 * _JSONcmd.h
 *
 *  Created on: May 6, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Protocol__JSONcmd_H_
#define OpenKAI_src_Protocol__JSONcmd_H_

#include "_JSONbase.h"

namespace kai
{

	class _JSONcmd : public _JSONbase
	{
	public:
		_JSONcmd();
		virtual ~_JSONcmd();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		bool start(void) override;
		void stop(void) override;
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		void handleJson(const string &str);

	private:
		// main thread
		void send(void);
		void sendHeartbeat(void);
		void sendConfig(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_JSONcmd *)This)->update();
			return NULL;
		}

		// UI handler
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_JSONcmd *)This)->updateR();
			return NULL;
		}

		BASE* findModule(const string& n);

	protected:
		vector<BASE *> m_vpB;
	};

}
#endif
