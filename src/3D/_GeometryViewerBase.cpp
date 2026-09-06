/*
 * _GeometryViewerBase.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_GeometryViewerBase.h"

namespace kai
{

	_GeometryViewerBase::_GeometryViewerBase()
	{
		m_vWinSize.set(1280, 720);
		m_bFullScreen = false;
		m_dirSave = "/home/lab/";

		m_vCoR.set(0, 0, 0);

		m_dTexpire = 0;
	}

	_GeometryViewerBase::~_GeometryViewerBase()
	{
	}

	bool _GeometryViewerBase::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		jKv<int>(j, "vWinSize", m_vWinSize);
		jKv(j, "bFullScreen", m_bFullScreen);
		jKv(j, "dirSave", m_dirSave);

		jKv(j, "camProjType", m_camProj.m_type);
		jKv(j, "camFov", m_camProj.m_fov);
		jKv<float>(j, "vCamLR", m_camProj.m_vLR);
		jKv<float>(j, "vCamBT", m_camProj.m_vBT);
		jKv<float>(j, "vCamNF", m_camProj.m_vNF);

		jKv<float>(j, "vCamLookAt", m_camPose.m_vLookAt);
		jKv<float>(j, "vCamEye", m_camPose.m_vEye);
		jKv<float>(j, "vCamUp", m_camPose.m_vUp);
		m_camPoseDefault;

		jKv<float>(j, "vCoR", m_vCoR);
		jKv(j, "dTexpire", m_dTexpire);

		return true;
	}

	bool _GeometryViewerBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_GeometryBase::link(j, pM));

		const json &jg = jK(j, "vGeometry");
		IF__(!jg.is_array(), true);

		for (auto it = jg.begin(); it != jg.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			string n = "";
			jKv(Ji, "_GeometryBase", n);
			_GeometryBase *pGB = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pGB);

			m_vpGb.push_back(pGB);
		}

		return true;
	}

	bool _GeometryViewerBase::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->startThread(getUpdate, this));

		return true;
	}

	bool _GeometryViewerBase::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _GeometryViewerBase::update(void)
	{
		// wait for the UI thread to get window ready
		m_pT->sleepT(USEC_1SEC);

		resetCamPose();
		updateCamPose();

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateAllGeometries();

			// update front end in the inherited class
		}
	}

	void _GeometryViewerBase::updateAllGeometries(void)
	{
		IF_(!check());

		for (_GeometryBase* pGb :m_vpGb)
		{
			m_grPt.clear();
			int nP = pGb->get(&m_grPt, m_dTexpire);
			// update the point cloud buffer to frontend ingerited class

			m_grLn.clear();
			int nL = pGb->get(&m_grLn, m_dTexpire);
			// update the line buffer to frontend ingerited class
		}
	}

	void _GeometryViewerBase::resetCamPose(void)
	{
		m_camPose = m_camPoseDefault;
	}

	void _GeometryViewerBase::setCamPose(const GVIEWER_CAM_POSE &camPose)
	{
		m_camPose = camPose;
		updateCamPose();
	}

	GVIEWER_CAM_POSE _GeometryViewerBase::getCamPose(void)
	{
		return m_camPose;
	}

	void _GeometryViewerBase::updateCamProj(void)
	{
		IF_(!check());

		// update camera projection parameters by m_camProj
	}

	void _GeometryViewerBase::updateCamPose(void)
	{
		IF_(!check());

		// update camera pose parameters by m_camProj
	}

}
