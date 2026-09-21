/*
 * _VisionBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_VisionBase.h"

namespace kai
{

	_VisionBase::_VisionBase()
	{
	}

	_VisionBase::~_VisionBase()
	{
	}

	bool _VisionBase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "devURI", m_devURI);
		jKv(j, "devFPS", m_devFPS);
		jKv(j, "bRGB", m_bRGB);
		jKv<int>(j, "vSizeRGB", m_vSizeRGB);

		return true;
	}

	bool _VisionBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		return true;
	}

	bool _VisionBase::open(void)
	{
		return false;
	}

	bool _VisionBase::bOpened(void)
	{
		return m_bOpened;
	}

	void _VisionBase::close(void)
	{
		m_bOpened = false;
	}

	bool _VisionBase::check(void)
	{
		return _ModuleBase::check();
	}

	Mat *_VisionBase::getMatRGB(void)
	{
		return &m_mRGB;
	}

	void _VisionBase::copyMatRGB(Mat &m)
	{
		std::lock_guard<std::mutex> lock(m_mutexRGB);
		m_mRGB.copyTo(m);
	}

	Vector2i _VisionBase::getSizeRGB(void)
	{
		return m_vSizeRGB;
	}

	VISION_TYPE _VisionBase::getType(void)
	{
		return m_type;
	}

	void _VisionBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

	void _VisionBase::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;

		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "saveImg")
		{
			string fName;
			IF_(!jKv(j, "fNameImg", fName));

			Mat m;
			{
				std::lock_guard<std::mutex> lock(m_mutexRGB);
				m_mRGB.copyTo(m);
			}
			IF_(m.empty());

			bool bR;
			if (m.type() == CV_8UC3)
			{
				bR = imwrite(fName, m);
			}

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "saveImg";
			jr["bSuccess"] = bR;
		}
	}

#ifdef USE_OPENCV
	void _VisionBase::draw(void *pMat)
	{
		NULL_(pMat);
		this->_ModuleBase::draw(pMat);
		IF_(!check());

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		IF_(m_mRGB.empty());
		Mat *pM = static_cast<Mat *>(pMat);
		m_mRGB.copyTo(*pM);
	}
#endif

}
