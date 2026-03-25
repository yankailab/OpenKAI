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
		m_type = vision_unknown;
		m_devURI = "";
		m_devFPS = 30;
		m_tFrameInterval = 0;
		m_bRGB = true;
		m_vSizeRGB.set(1280, 720);
		m_bOpen = false;
		m_psmRGB = nullptr;
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

		string n = "";
		jKv(j, "_SHMrgb", n);
		m_psmRGB = (SharedMem *)(pM->findModule(n));

		return true;
	}

	bool _VisionBase::open(void)
	{
		return false;
	}

	bool _VisionBase::isOpened(void)
	{
		return m_bOpen;
	}

	void _VisionBase::close(void)
	{
		m_bOpen = false;
	}

	bool _VisionBase::check(void)
	{
		return _ModuleBase::check();
	}

	Frame *_VisionBase::getFrameRGB(void)
	{
		return &m_fRGB;
	}

	vInt2 _VisionBase::getSize(void)
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

			Mat m = *m_fRGB.m();
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
	void _VisionBase::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(!check());
		IF_(m_fRGB.bEmpty());

		Frame *pF = (Frame *)pFrame;
		pF->copy(m_fRGB);
	}
#endif

}
