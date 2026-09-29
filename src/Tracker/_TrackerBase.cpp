/*
 * _TrackerBase.cpp
 *
 *  Created on: Aug 28, 2018
 *      Author: yankai
 */

#include "_TrackerBase.h"
#include "../UI/_Console.h"

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

	bool _TrackerBase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(m_pRGBin == nullptr, "RGBframeIn not found: " + n);

		n = "";
		jKv(j, "BBoxStreamOut", n);
		m_pBBout = dynamic_cast<BBoxStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(m_pBBout == nullptr, "BBoxStreamOut not found: " + n);

		return true;
	}

	void _TrackerBase::createTracker(void)
	{
	}

	bool _TrackerBase::check(void)
	{
		NULL_F(m_pRGBin);
		NULL_F(m_pBBout);

		return this->_ModuleBase::check();
	}

	void _TrackerBase::update(void)
	{
	}

	void _TrackerBase::stopTrack(void)
	{
		std::lock_guard lock(m_mutex);
		m_trackState = track_stop;
		m_iInit = m_iSet; // Cancel a pending initialization, too.
		m_bb.setZero();
	}

	bool _TrackerBase::startTrack(const Vector4f &bb)
	{
		NULL_F(m_pRGBin);
		NULL_F(m_pBBout);
		IF_F(!bb.allFinite());
		std::lock_guard lock(m_mutex);
		Mat image;
		m_pRGBin->get(image);
		IF_F(image.empty());

		float mBig = 1.0 + m_margin;
		float mSmall = 1.0 - m_margin;
		Vector4f target = bb;
		target.x() = constrain(target.x() * mSmall, 0.0f, 1.0f);
		target.y() = constrain(target.y() * mSmall, 0.0f, 1.0f);
		target.z() = constrain(target.z() * mBig, 0.0f, 1.0f);
		target.w() = constrain(target.w() * mBig, 0.0f, 1.0f);
		IF_F(target.z() <= target.x() || target.w() <= target.y());

		Rect rBB = bb2Rect(bbScale(target, image.cols, image.rows));
		IF_F(rBB.width <= 0 || rBB.height <= 0);
		m_newBB = rBB;
		m_iSet++;
		m_trackState = track_init;
		return true;
	}

	void _TrackerBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		std::lock_guard lock(m_mutex);
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
		IF_(!check());

		Mat *pM = static_cast<Mat *>(pMat);
		IF_(pM->empty());

		std::lock_guard lock(m_mutex);
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
