/*
 * _Uploader.h
 *
 *  Created on: May 30, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Net__Uploader_H_
#define OpenKAI_src_Net__Uploader_H_

#include "../Data/_FileBase.h"
#include "../Module/JSON.h"
#include "../UI/_Console.h"

#include "base64.h"
#include "HttpClient.h"

#define UPLOADER_NB 10240000

namespace kai
{
	enum UPLOADER_METHOD
	{
		uploader_http = 0,
		uploader_curl = 1,
	};

	class _Uploader : public _FileBase
	{
	public:
		_Uploader(void);
		virtual ~_Uploader();

		virtual int init(void *pKiss);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		bool addFile(const string &fName);

	private:
		bool uploadByCurl(const string& fName);
		bool uploadByHttp(const string& url, const string& fName);

		bool bFileInList(const string &fName);
		void updateFileList(void);
		void updateUpload(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Uploader *)This)->update();
			return NULL;
		}

	private:
		string m_dir;
		vector<string> m_vFiles;
		string m_fName;
		bool m_bRemoveAfterUpload;

		UPLOADER_METHOD m_method;
		string m_url;
		uint8_t m_pBuf[UPLOADER_NB];

		// Curl etc
		string m_cmd;
		bool m_bConfirmCmdResult;

		// Http
		HttpClient m_httpC;

	};

}
#endif
