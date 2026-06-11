/*
 * _Frame2Img.cpp
 *
 *  Created on: June 11, 2026
 *      Author: yankai
 */

#include "_Frame2Img.h"

namespace kai
{
	static string frame2ImgTimeStr(void)
	{
		time_t timer;
		char buffer[18];
		struct tm *tm_info;

		time(&timer);
		tm_info = localtime(&timer);
		strftime(buffer, sizeof(buffer), "%y-%m-%d_%H-%M-%S", tm_info);

		return string(buffer);
	}

	_Frame2Img::_Frame2Img()
	{
		m_type = vision_Frame2Img;
		m_pV = nullptr;

		m_dir = "";
		m_dirRec = "";
		m_tRec = "";
		m_iFrame = 0;
		m_pngCompression = 3;
		m_vOffset = 50.0;
		m_vScale = 64.0;
	}

	_Frame2Img::~_Frame2Img()
	{
	}

	bool _Frame2Img::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "dir", m_dir);
		jKv(j, "pngCompression", m_pngCompression);
		jKv(j, "vOffset", m_vOffset);
		jKv(j, "vScale", m_vScale);

		if (!m_dir.empty())
			m_dir = checkDirName(m_dir);

		m_tRec = frame2ImgTimeStr();
		m_dirRec = m_dir + m_tRec;
		m_iFrame = 0;
		m_pngCompression = max(0, min(9, m_pngCompression));

		IF_F((mkdir(m_dirRec.c_str(), 0777) != 0) && (errno != EEXIST));
		m_dirRec = checkDirName(m_dirRec);

		return true;
	}

	bool _Frame2Img::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Frame2Img::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Frame2Img::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			recFrame();
		}
	}

	void _Frame2Img::recFrame(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);

		Mat mT = *pF->m();
		IF_(mT.empty());
		IF_(mT.type() != CV_32FC1);

		Mat mC = (mT + m_vOffset) * m_vScale;
		Mat mOut;
		mC.convertTo(mOut, CV_16UC1);

		vector<int> vParam;
		vParam.push_back(IMWRITE_PNG_COMPRESSION);
		vParam.push_back(m_pngCompression);

		string fName = m_dirRec + m_tRec + "_" + i2str(m_iFrame) + ".png";
		IF_(!imwrite(fName, mOut, vParam));
		m_iFrame++;

	}

	void _Frame2Img::console(const json &j, void *pJSONbase)
	{
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		// if (cmd == "setThermal")
		// {
		// 	jKv<float>(j, "vTrange", m_vTrange);
		// }

		this->_VisionBase::console(j, pJSONbase);
	}

}
