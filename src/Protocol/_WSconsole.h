/*
 * _WSconsole.h
 *
 *  Created on: May 6, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Protocol__WSconsole_H_
#define OpenKAI_src_Protocol__WSconsole_H_

#include "_JSONbase.h"

namespace kai
{

	class _WSconsole : public _JSONbase
	{
	public:
		_WSconsole();
		virtual ~_WSconsole();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
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
			((_WSconsole *)This)->update();
			return NULL;
		}

		// UI handler
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_WSconsole *)This)->updateR();
			return NULL;
		}

		BASE* findModule(const string& n);

	protected:
		vector<BASE *> m_vpB;
	};

}
#endif
