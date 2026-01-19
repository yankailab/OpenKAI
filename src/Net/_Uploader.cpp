/*
 * _Uploader.cpp
 *
 *  Created on: May 30, 2022
 *      Author: yankai
 */

#include "_Uploader.h"

namespace kai
{
	_Uploader::_Uploader()
	{
		m_dir = "";
		m_fName = "";
		m_bRemoveAfterUpload = true;
		m_method = uploader_http;
		m_url = "";
		m_cmd = "";
		m_bConfirmCmdResult = false;
	}

	_Uploader::~_Uploader()
	{
	}

	bool _Uploader::init(const json &j)
	{
		IF_F(!this->_FileBase::init(j));

		jKv(j, "dir", m_dir);
		m_dir = checkDirName(m_dir);
		jKv(j, "bRemoveAfterUpload", m_bRemoveAfterUpload);
		jKv(j, "method", m_method);
		jKv(j, "url", m_url);
		jKv(j, "cmd", m_cmd);
		jKv(j, "bConfirmCmdResult", m_bConfirmCmdResult);

		if (m_method == uploader_http)
		{
			IF_F(m_httpC.init());
		}

		return true;
	}

	bool _Uploader::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _Uploader::check(void)
	{
		return this->_FileBase::check();
	}

	void _Uploader::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateFileList();
			updateUpload();
		}
	}

	void _Uploader::updateFileList(void)
	{
		vector<string> vFile;
		IF_(!getDirFileList(m_dir, &vFile, &m_vExt));

		for (string f : vFile)
		{
			addFile(f);
		}
	}

	void _Uploader::updateUpload(void)
	{
		IF_(!check());

		while (!m_vFiles.empty())
		{
			m_fName = m_vFiles.back();

			bool bR;
			switch (m_method)
			{
			case uploader_http:
				bR = uploadByHttp(m_url, m_fName);
				break;

			case uploader_curl:
				bR = uploadByCurl(m_fName);
				break;
			}

			if (bR && m_bRemoveAfterUpload)
			{
				// upload success
				LOG_I("Uploaded: " + m_fName);
				string cmd = "rm " + m_fName;
				system(cmd.c_str());
				m_vFiles.pop_back();
			}

			m_fName = "";
		}
	}

	bool _Uploader::uploadByCurl(const string &fName)
	{
		string cmd = replace(m_cmd, "[fName]", fName);

		FILE *fp;
		fp = popen(cmd.c_str(), "r");
		if (fp == nullptr)
		{
			LOG_E("Failed to run command: " + cmd);
			return false;
		}

		string strR = "";
		char pResult[1035];
		while (fgets(pResult, sizeof(pResult), fp))
		{
			strR += string(pResult);
		}
		pclose(fp);
		LOG_I(strR);

		bool bR = true;
		if (m_bConfirmCmdResult)
		{
			bR = strR.empty();
		}

		return bR;
	}

	bool _Uploader::uploadByHttp(const string &url, const string &fName)
	{
		FILE *fp = fopen(fName.c_str(), "r");
		IF_F(!fp);

		size_t nB = fread(m_pBuf, 1, UPLOADER_NB, fp);
		fclose(fp);

		string strEnc = base64_encode(m_pBuf, nB, false);

		// string strDec = base64_decode(strEnc, false);
		// int len = strDec.length();
		// string fOut = fName + ".png";
		// fp = fopen(fOut.c_str(), "w");
		// fwrite(strDec.c_str(), len, 1,fp);
		// fflush(fp);
		// fclose(fp);

		// object o;
		// JO(o, "id", "1");
		// JO(o, "ip", "192.168.1.108");
		// JO(o, "longitude", lf2str(123.456, 10));
		// JO(o, "latitude", lf2str(654.321, 10));
		// JO(o, "img", strEnc);
		// JO(o, "status", (double)3);
		// string jsonStr = picojson::value(o).serialize();

		// m_httpC.post_imageinfo(m_url.c_str(), jsonStr.c_str());

		return true;
	}

	bool _Uploader::addFile(const string &fName)
	{
		IF_F(fName.empty());
		IF__(bFileInList(fName), true);

		m_vFiles.push_back(fName);
		return true;
	}

	bool _Uploader::bFileInList(const string &fName)
	{
		for (string f : m_vFiles)
		{
			IF__(f == fName, true);
		}

		return false;
	}

	void _Uploader::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_FileBase::console(pConsole);
		IF_(!check());

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("Uploading: " + m_fName);
	}

}
