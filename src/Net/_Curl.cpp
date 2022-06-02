/*
 * _Curl.cpp
 *
 *  Created on: May 30, 2022
 *      Author: yankai
 */

#include "_Curl.h"

namespace kai
{
	_Curl::_Curl()
	{
		m_cmd = "";
	}

	_Curl::~_Curl()
	{
	}

	bool _Curl::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("cmd", &m_cmd);

		string n;

		return true;
	}

	bool _Curl::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _Curl::check(void)
	{
		return 0;
	}

	void _Curl::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			addFile("/home/kai/Videos/test.mka");
			updateCurl();

			m_pT->autoFPSto();
		}
	}

	void _Curl::updateCurl(void)
	{
		while (!m_vFiles.empty())
		{
			string fName = m_vFiles.back();
			string cmd = replace(m_cmd, "[fName]", fName);

			FILE *fp;
			fp = popen(cmd.c_str(), "r");
			if (!fp)
			{
				LOG_E("Failed to run command: " + cmd);
				continue;
			}

			string strR = "";
			char pResult[1035];
			while (fgets(pResult, sizeof(pResult), fp))
			{
				strR += string(pResult);
			}
			pclose(fp);

			LOG_I(strR);
			if (strR.empty())
			{
				//upload success
				cmd = "rm " + fName;
				system(cmd.c_str());
				m_vFiles.pop_back();
			}
		}
	}

	bool _Curl::addFile(const string &fName)
	{
		IF_F(fName.empty());

		m_vFiles.push_back(fName);
		return true;
	}

	void _Curl::console(void *pConsole)
	{
		this->_ModuleBase::console(pConsole);
	}

}
