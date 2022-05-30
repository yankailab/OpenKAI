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
	
		m_dir = "";
	}

	_Curl::~_Curl()
	{
	}

	bool _Curl::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("dir", &m_dir);

		string n;

		return true;
	}

	bool _Curl::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
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

	int _Curl::check(void)
	{
		return 0;
	}

	void _Curl::updateCurl(void)
	{
		FILE *fp;
		char path[1035];
		string alpr = "curl -X POST -F image=@" + m_tempDir + fName + ".jpeg '" + m_alprAPI + m_alprKey + "'";
		fp = popen(alpr.c_str(), "r");
		IF_l(!fp, "Failed to run command:" + alpr);

		while (fgets(path, sizeof(path) - 1, fp))
			;
		pclose(fp);
		string strR = string(path);

		string file = m_tempDir + fName + ".txt";
		m_fAlpr.open(file.c_str(), ios::out);
		IF_(!m_fAlpr.is_open());
		m_fAlpr.seekg(0, ios_base::beg);

		IF_(!m_fAlpr.write((char *)strR.c_str(), strR.length()));

		m_fAlpr.flush();
		m_fAlpr.close();
	}

	void _Curl::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = pFrame;
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		Scalar col = Scalar(0, 255, 0);

		vFloat4 roi;
		roi.x = m_vRoi.x * pM->cols;
		roi.y = m_vRoi.y * pM->rows;
		roi.z = m_vRoi.z * pM->cols;
		roi.w = m_vRoi.w * pM->rows;

		Rect r = bb2Rect(roi);
		rectangle(*pM, r, col, 3);
	}

}
