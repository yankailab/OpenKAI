/*
 * _TrackerBase.cpp
 *
 *  Created on: Aug 28, 2018
 *      Author: yankai
 */

#include "_TrackerBase.h"

namespace kai
{

	_TrackerBase::_TrackerBase()
	{
		m_pV = nullptr;
		m_trackerType = "";
		m_trackState = track_stop;
		m_bb.clear();
		m_iSet = 0;
		m_iInit = 0;
		m_margin = 0.0;
	}

	_TrackerBase::~_TrackerBase()
	{
	}

	bool _TrackerBase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "trackerType", m_trackerType);
		jKv(j, "margin", m_margin);

		return true;
	}

	bool _TrackerBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	void _TrackerBase::createTracker(void)
	{
	}

	bool _TrackerBase::check(void)
	{
		NULL_F(m_pV);

		return this->_ModuleBase::check();
	}

	void _TrackerBase::update(void)
	{
	}

	void _TrackerBase::stopTrack(void)
	{
		m_trackState = track_stop;
	}

	TRACK_STATE _TrackerBase::trackState(void)
	{
		return m_trackState;
	}

	vFloat4 *_TrackerBase::getBB(void)
	{
		return &m_bb;
	}

	bool _TrackerBase::startTrack(vFloat4 &bb)
	{
		NULL_F(m_pV);
		Mat *pM = m_pV->getFrameRGB()->m();
		IF_F(pM->empty());

		float mBig = 1.0 + m_margin;
		float mSmall = 1.0 - m_margin;

		bb.x = constrain(bb.x * mSmall, 0.0f, 1.0f);
		bb.y = constrain(bb.y * mSmall, 0.0f, 1.0f);
		bb.z = constrain(bb.z * mBig, 0.0f, 1.0f);
		bb.w = constrain(bb.w * mBig, 0.0f, 1.0f);

		Rect rBB = bb2Rect(bbScale(bb, pM->cols, pM->rows));
		IF_F(rBB.width == 0 || rBB.height == 0);

		m_newBB = rBB;

		m_iSet++;
		m_trackState = track_init;
		return true;
	}

	void _TrackerBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg = "Stop";
		if (m_trackState == track_init)
			msg = "Init";
		else if (m_trackState == track_update)
			msg = "Update";

		pC->addMsg(msg, 1);
		pC->addMsg("Tracking pos = (" + f2str(m_bb.midX()) + ", " + f2str(m_bb.midY()) + ")");
	}

	void _TrackerBase::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(!check());

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		Scalar col;
		if (m_trackState == track_init)
		{
			col = Scalar(0, 255, 255);
			rectangle(*pM, m_newBB, col, 2);
		}
		else if (m_trackState == track_update)
		{
			col = Scalar(0, 255, 0);
			rectangle(*pM, m_rBB, col, 2);
		}
	}

}
