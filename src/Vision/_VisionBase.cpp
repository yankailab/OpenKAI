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
		m_bOpen = false;
		m_psmRGB = nullptr;
		m_tFrameInterval = 0;
	}

	_VisionBase::~_VisionBase()
	{
	}

	bool _VisionBase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		m_devURI = j.value("devURI", "");
		m_devFPS = j.value("devFPS", 30);
		m_bRGB = j.value("bRGB", true);
		m_vSizeRGB = j.value("vSizeRGB", vector<int>{1280, 720});

		return true;
	}

	bool _VisionBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = j.value("_SHMrgb", "");
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

#ifdef USE_OPENCV
	void _VisionBase::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(!check());
		IF_(m_fRGB.bEmpty());

		Frame *pF = (Frame *)pFrame;
		pF->copy(m_fRGB);

		// Mat *pM = pF->m();
		// IF_(pM->empty());
		// Rect r = bb2Rect(bbScale(m_bbDraw, pM->cols, pM->rows));
		// Mat m;
		// cv::resize(*m_fRGB.m(), m, Size(r.width, r.height));
		// m.copyTo((*pM)(r));
	}
#endif

}
