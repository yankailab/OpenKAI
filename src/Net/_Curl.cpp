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

			updateCurl();

			m_pT->autoFPSto();
		}
	}

	void _Curl::updateCurl(void)
	{
		// FILE *fp;
		// char path[1035];
		// string alpr = "curl -X POST -F image=@" + m_tempDir + fName + ".jpeg '" + m_alprAPI + m_alprKey + "'";
		// fp = popen(alpr.c_str(), "r");
		// IF_l(!fp, "Failed to run command:" + alpr);

		// while (fgets(path, sizeof(path) - 1, fp));
		// pclose(fp);
		// string strR = string(path);
	}

    void _Curl::console(void *pConsole)
    {
        this->_ModuleBase::console(pConsole);
    }

}
