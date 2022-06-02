/*
 * _Curl.h
 *
 *  Created on: May 30, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Net__Curl_H_
#define OpenKAI_src_Net__Curl_H_

#include "../Base/_ModuleBase.h"
#include "../Script/JSON.h"
#include "../UI/_Console.h"

namespace kai
{

	class _Curl : public _ModuleBase
	{
	public:
		_Curl(void);
		virtual ~_Curl();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		bool addFile(const string& fName);

	private:
		void updateCurl(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Curl *)This)->update();
			return NULL;
		}

	private:
		string m_cmd;
		vector<string> m_vFiles;
		string m_fName;
	};

}
#endif
