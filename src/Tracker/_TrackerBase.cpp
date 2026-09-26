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
		m_bb.setZero();
	}

	_TrackerBase::~_TrackerBase()
	{
	}

	bool _TrackerBase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "trackerType", m_trackerType);
		jKv(j, "margin", m_margin);

		return true;
	}

	bool _TrackerBase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["trackerType"] = m_trackerType;
		j["margin"] = m_margin;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _TrackerBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
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

	Vector4f *_TrackerBase::getBB(void)
	{
		return &m_bb;
	}

	bool _TrackerBase::startTrack(Vector4f &bb)
	{
		NULL_F(m_pV);
		Mat *pM = m_pV->getMatRGB();
		IF_F(pM->empty());

		float mBig = 1.0 + m_margin;
		float mSmall = 1.0 - m_margin;

		bb.x() = constrain(bb.x() * mSmall, 0.0f, 1.0f);
		bb.y() = constrain(bb.y() * mSmall, 0.0f, 1.0f);
		bb.z() = constrain(bb.z() * mBig, 0.0f, 1.0f);
		bb.w() = constrain(bb.w() * mBig, 0.0f, 1.0f);

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
		pC->addMsg("Tracking pos = (" + f2str(((m_bb.x() + m_bb.z()) / 2)) + ", " + f2str(((m_bb.y() + m_bb.w()) / 2)) + ")");
	}

	void _TrackerBase::draw(void *pMat)
	{
		NULL_(pMat);
		this->_ModuleBase::draw(pMat);
		IF_(!check());

		Mat *pM = static_cast<Mat *>(pMat);
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
