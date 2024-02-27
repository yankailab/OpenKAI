/*
 * _Curl.h
 *
 *  Created on: May 30, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Net__Curl_H_
#define OpenKAI_src_Net__Curl_H_

#include "../File/_FileBase.h"
#include "../Script/JSON.h"
#include "../UI/_Console.h"

namespace kai
{

	class _Curl : public _FileBase
	{
	public:
		_Curl(void);
		virtual ~_Curl();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		bool addFile(const string &fName);

	private:
		bool bFileInList(const string &fName);
		void updateFileList(void);
		void updateCurl(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Curl *)This)->update();
			return NULL;
		}

	private:
		string m_cmd;
		bool m_bConfirmCmdResult;

		string m_dir;
		vector<string> m_vFiles;
		string m_fName;
	};

}
#endif
