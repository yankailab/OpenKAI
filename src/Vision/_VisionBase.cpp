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

	int _VisionBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("devURI", &m_devURI);
		pK->v("devFPS", &m_devFPS);
		pK->v("tFrameInterval", &m_tFrameInterval);
		pK->v("bRGB", &m_bRGB);
		pK->v("vSizeRGB", &m_vSizeRGB);

		return OK_OK;
	}

	int _VisionBase::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		n = "";
		pK->v("_SHMrgb", &n);
		m_psmRGB = (SharedMem *)(pK->findModule(n));

		return OK_OK;
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

	int _VisionBase::check(void)
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
		IF_(check() != OK_OK);
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
